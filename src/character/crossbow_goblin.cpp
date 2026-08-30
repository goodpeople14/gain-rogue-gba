#include "character/crossbow_goblin.h"

#include "bn_assert.h"
#include "bn_sprite_items_crossbow_goblin.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin_awareness_icons.h"
#include "bn_sprite_items_goblin_recovery_hourglass.h"

#include "combat/collision/collision_math.h"
#include "combat/crossbow_projectile_pool.h"
#include "combat/hit_effect_manager.h"

#if defined(GAIN_PERF_DEBUG_LOGS)
    #include "debug/perf_stats.h"
#endif
#include "combat/melee/swordsman_attack.h"

namespace
{
    constexpr CharacterDefinition crossbow_goblin_definition = {
        CharacterId::CROSSBOW_GOBLIN, "CROSSBOW GOBLIN", 15, 1
    };
    constexpr int home_radius = 28;
    // A ranged goblin must notice and retain a target beyond its 4-5 cell firing band.
    constexpr int discovery_distance = 96;
    constexpr int disengage_distance = 110;
    constexpr int roam_direction_frames = 42;
    constexpr int telegraph_frames = 90;
    constexpr int recovery_frames = 28;
    constexpr int discovery_flash_frames = 24;
    constexpr int discovery_bulb_white_frames = 10;
    constexpr int return_question_frames = 21;
    constexpr int respawn_delay_ticks = 120;
    constexpr int goblin_size = 16;
    constexpr int flee_radius = 48;
    constexpr int ranged_commit_radius = 72;

    static_assert(crossbow_goblin_definition.id == CharacterId::CROSSBOW_GOBLIN);
    static_assert(crossbow_goblin_definition.display_name_length == 15);
    static_assert(crossbow_goblin_definition.max_hp == 1);
    constexpr bn::fixed roam_speed = bn::fixed(1) / 4;
    constexpr bn::fixed chase_speed = bn::fixed(1) / 2;
    constexpr bn::fixed diagonal_ratio(0.70710678f);
    constexpr MovementBounds movement_bounds = Battlefield::movement_bounds(goblin_size, goblin_size);
    constexpr Pushbox crossbow_body_pushbox = { { 0, 3, 6, 6 } };
    constexpr CollisionBody crossbow_collision_body = { { { 0, 1, 8, 10 } }, crossbow_body_pushbox };

    static_assert(crossbow_body_pushbox.box.offset_y == 3);
    static_assert(crossbow_body_pushbox.box.width == 6 && crossbow_body_pushbox.box.height == 6);
    constexpr bn::array<Direction, 4> roam_directions = {
        Direction::RIGHT, Direction::DOWN, Direction::LEFT, Direction::UP
    };

    enum class RangedDecision
    {
        APPROACH,
        HOLD,
        RETREAT
    };

    struct RangedAttackTick
    {
        CrossbowGoblin::State next_state;
        int next_timer;
        bool spawn_projectile;
    };

    [[nodiscard]] constexpr int sign(bn::fixed value)
    {
        return value > 0 ? 1 : value < 0 ? -1 : 0;
    }

    [[nodiscard]] constexpr Direction direction_from_components(
            int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical) return fallback;
        if(vertical > 0) return horizontal < 0 ? Direction::DOWN_LEFT : horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN;
        if(vertical < 0) return horizontal < 0 ? Direction::UP_LEFT : horizontal > 0 ? Direction::UP_RIGHT : Direction::UP;
        return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
    }

    [[nodiscard]] constexpr RangedDecision ranged_decision(
            const bn::fixed_point& enemy_foot, const bn::fixed_point& player_foot)
    {
        // The Flee boundary itself belongs to HOLD, so only its strict interior retreats.
        if(Enemy::within_distance_strict(enemy_foot, player_foot, flee_radius))
        {
            return RangedDecision::RETREAT;
        }

        if(Enemy::within_distance(enemy_foot, player_foot, ranged_commit_radius))
        {
            return RangedDecision::HOLD;
        }

        return RangedDecision::APPROACH;
    }

    [[nodiscard]] constexpr RangedAttackTick ranged_attack_tick(
            CrossbowGoblin::State state, int timer)
    {
        if(state == CrossbowGoblin::State::TELEGRAPH)
        {
            if(timer == 1)
            {
                return { CrossbowGoblin::State::RECOVERY, recovery_frames, true };
            }
            return { state, timer - 1, false };
        }

        if(state == CrossbowGoblin::State::RECOVERY)
        {
            return timer == 1 ? RangedAttackTick{ CrossbowGoblin::State::CHASE, 0, false } :
                    RangedAttackTick{ state, timer - 1, false };
        }

        return { state, timer, false };
    }

    [[nodiscard]] constexpr bool ranged_attack_lifecycle_is_stable()
    {
        CrossbowGoblin::State state = CrossbowGoblin::State::TELEGRAPH;
        int timer = telegraph_frames;
        int projectile_spawns = 0;
        for(int frame = 0; frame < telegraph_frames + recovery_frames; ++frame)
        {
            RangedAttackTick tick = ranged_attack_tick(state, timer);
            state = tick.next_state;
            timer = tick.next_timer;
            projectile_spawns += tick.spawn_projectile ? 1 : 0;
        }
        return state == CrossbowGoblin::State::CHASE && timer == 0 && projectile_spawns == 1;
    }

    void direction_components(Direction direction, int& horizontal, int& vertical)
    {
        static constexpr bn::array<int, 8> horizontal_components = { 0, -1, -1, -1, 0, 1, 1, 1 };
        static constexpr bn::array<int, 8> vertical_components = { 1, 1, 0, -1, -1, -1, 0, 1 };
        horizontal = horizontal_components[int(direction)];
        vertical = vertical_components[int(direction)];
    }

    [[nodiscard]] bn::fixed_point directional_offset(Direction direction, int distance)
    {
        int horizontal;
        int vertical;
        direction_components(direction, horizontal, vertical);
        bn::fixed step = (horizontal && vertical) ? bn::fixed(distance) * diagonal_ratio : distance;
        return { horizontal * step, vertical * step };
    }

    [[nodiscard]] constexpr int discovery_bulb_frame(int remaining)
    {
        return remaining > discovery_flash_frames - discovery_bulb_white_frames ? 0 : 1;
    }

    [[nodiscard]] constexpr int recovery_hourglass_frame(int remaining) { return remaining > recovery_frames / 2 ? 0 : 1; }

    [[nodiscard]] bn::fixed_point status_icon_position(const bn::fixed_point& position)
    {
        bn::fixed y = position.y() - 13;
        return { position.x(), y < -70 ? -70 : y };
    }

    static_assert(ranged_decision({ 0, 0 }, { 0, 0 }) == RangedDecision::RETREAT);
    static_assert(ranged_decision({ 0, 0 }, { flee_radius - 1, 0 }) ==
                  RangedDecision::RETREAT);
    static_assert(ranged_decision({ 0, 0 }, { 40, 40 }) == RangedDecision::HOLD);
    static_assert(ranged_decision({ 0, 0 }, { flee_radius, 0 }) == RangedDecision::HOLD);
    static_assert(ranged_decision({ 0, 0 }, { flee_radius + 1, 0 }) == RangedDecision::HOLD);
    static_assert(ranged_decision({ 0, 0 }, { ranged_commit_radius, 0 }) ==
                  RangedDecision::HOLD);
    static_assert(ranged_decision({ 0, 0 }, { ranged_commit_radius, ranged_commit_radius }) ==
                  RangedDecision::APPROACH);
    static_assert(ranged_decision({ 0, 0 }, { ranged_commit_radius + 1, 0 }) ==
                  RangedDecision::APPROACH);
    static_assert(ranged_decision({ 0, 0 }, { 0, ranged_commit_radius + 1 }) ==
                  RangedDecision::APPROACH);
    static_assert(direction_from_components(-1, -1, Direction::DOWN) == Direction::UP_LEFT);
    static_assert(direction_from_components(0, -1, Direction::DOWN) == Direction::UP);
    static_assert(direction_from_components(1, -1, Direction::DOWN) == Direction::UP_RIGHT);
    static_assert(direction_from_components(-1, 0, Direction::DOWN) == Direction::LEFT);
    static_assert(direction_from_components(1, 0, Direction::DOWN) == Direction::RIGHT);
    static_assert(direction_from_components(-1, 1, Direction::UP) == Direction::DOWN_LEFT);
    static_assert(direction_from_components(0, 1, Direction::UP) == Direction::DOWN);
    static_assert(direction_from_components(1, 1, Direction::UP) == Direction::DOWN_RIGHT);
    static_assert(direction_from_components(0, 0, Direction::LEFT) == Direction::LEFT);
    static_assert(ranged_attack_tick(CrossbowGoblin::State::TELEGRAPH, telegraph_frames).next_timer ==
                  telegraph_frames - 1);
    static_assert(! ranged_attack_tick(CrossbowGoblin::State::TELEGRAPH, telegraph_frames).spawn_projectile);
    static_assert(ranged_attack_tick(CrossbowGoblin::State::TELEGRAPH, 1).next_state ==
                  CrossbowGoblin::State::RECOVERY);
    static_assert(ranged_attack_tick(CrossbowGoblin::State::TELEGRAPH, 1).spawn_projectile);
    static_assert(! ranged_attack_tick(CrossbowGoblin::State::RECOVERY, recovery_frames).spawn_projectile);
    static_assert(ranged_attack_tick(CrossbowGoblin::State::RECOVERY, 1).next_state ==
                  CrossbowGoblin::State::CHASE);
    static_assert(ranged_attack_lifecycle_is_stable());
    static_assert(Enemy::within_distance({ 0, 0 }, { discovery_distance, 0 }, discovery_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { discovery_distance + 1, 0 }, discovery_distance));
    static_assert(Enemy::within_distance({ 0, 0 }, { 67, 67 }, discovery_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { 68, 68 }, discovery_distance));
    static_assert(Enemy::within_distance({ 0, 0 }, { disengage_distance, 0 }, disengage_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { disengage_distance + 1, 0 }, disengage_distance));
    static_assert(world_foot_position({ 0, 0 }, crossbow_body_pushbox) == bn::fixed_point(0, 6));
}

CrossbowGoblin::CrossbowGoblin(const bn::fixed_point& home_position, int target_id) :
    Enemy(bn::sprite_items::crossbow_goblin, home_position, Direction::DOWN, chase_speed, crossbow_collision_body,
          crossbow_goblin_definition, movement_bounds, target_id),
    _locked_target(home_position),
    _awareness_icon_tiles{{ bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(0),
                             bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(1),
                             bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(2) }},
    _awareness_icon_palette(bn::sprite_items::goblin_awareness_icons.palette_item().create_palette()),
    _telegraph_tiles(bn::sprite_items::enemy_telegraph.tiles_item().create_tiles()),
    _telegraph_palette(bn::sprite_items::enemy_telegraph.palette_item().create_palette()),
    _recovery_hourglass_tiles{{ bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(0),
                                 bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(1) }},
    _recovery_hourglass_palette(bn::sprite_items::goblin_recovery_hourglass.palette_item().create_palette()),
    _status_icon_sprite(bn::sprite_items::goblin_awareness_icons.create_sprite(home_position, 0))
{
    _status_icon_sprite.set_z_order(-2);
    _status_icon_sprite.set_visible(false);
}

void CrossbowGoblin::enter()
{
    enter_enemy();
    _state = State::ROAM; _state_timer = roam_direction_frames; _roam_direction_index = 0;
    _attack_direction = Direction::DOWN; _locked_target = home_position();
    _status_icon = StatusIcon::NONE;
    _status_icon_timer = 0; _status_icon_frame = 0;
    _set_telegraph_visible(false);
}

void CrossbowGoblin::deactivate()
{
    deactivate_enemy();
    _state = State::DEAD;
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
}

void CrossbowGoblin::hide()
{
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
    hide_enemy();
}

void CrossbowGoblin::update(const WorldBox& player_hurtbox, const bn::fixed_point& player_foot_position,
                             const WorldBoxList<max_movement_obstacles>& blockers, CrossbowProjectilePool& projectiles)
{
    if(! active())
    {
        if(respawn_ready(blockers))
        {
            enter();
        }
        return;
    }
    _update_timed_status_icon();
    switch(_state)
    {
    case State::ROAM:
        if(within_distance(foot_position(), player_foot_position, discovery_distance))
        { _state = State::CHASE; _status_icon_timer = discovery_flash_frames; _set_awareness_icon(StatusIcon::DISCOVERY_FLASH); }
        else _update_roam(blockers);
        break;
    case State::CHASE: _update_chase(player_hurtbox, player_foot_position, blockers); break;
    case State::TELEGRAPH: _update_telegraph(player_hurtbox, projectiles); break;
    case State::RECOVERY:
    {
        _set_recovery_hourglass_visible(true);
        RangedAttackTick tick = ranged_attack_tick(_state, _state_timer);
        _state = tick.next_state;
        _state_timer = tick.next_timer;
        if(_state != State::RECOVERY)
        {
            _set_recovery_hourglass_visible(false);
        }
        break;
    }
    case State::RETURN: _update_return(player_foot_position, blockers); break;
    case State::DEAD: break;
    default: break;
    }
    if(_status_icon != StatusIcon::NONE) _status_icon_sprite.set_position(status_icon_position(position()));
}

void CrossbowGoblin::resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects)
{
    int damage = active() ? attack.try_hit(target_id(), position(), collision_body().hurtbox) : 0;
    if(damage > 0)
    {
        hit_effects.spawn(world_hurtbox().center);
        take_damage(damage);
        if(dead())
        {
            _die();
        }
    }
}

CrossbowGoblin::State CrossbowGoblin::state() const
{
    return _state;
}

void CrossbowGoblin::append_debug_shapes(
        CollisionDebugBoxList& boxes, CollisionDebugRadiusList& radii) const
{
    if(! active()) return;
    boxes.add(world_hurtbox(), CollisionDebugBoxType::HURTBOX); boxes.add(world_pushbox(), CollisionDebugBoxType::PUSHBOX);
    bn::fixed_point center = foot_position();
    radii.add(center, discovery_distance, CollisionDebugRadiusType::DISCOVERY);
    radii.add(center, disengage_distance, CollisionDebugRadiusType::DISENGAGE);
    radii.add(center, ranged_commit_radius, CollisionDebugRadiusType::RANGED_COMMIT);
    radii.add(center, flee_radius, CollisionDebugRadiusType::FLEE);
}

void CrossbowGoblin::_update_roam(const WorldBoxList<max_movement_obstacles>& blockers)
{
    move_direction(roam_directions[_roam_direction_index], roam_speed, blockers, true, home_radius);
    if(--_state_timer == 0) { _roam_direction_index = (_roam_direction_index + 1) % roam_directions.size(); _state_timer = roam_direction_frames; }
}

void CrossbowGoblin::_update_chase(
        const WorldBox& player_hurtbox, const bn::fixed_point& player_foot_position,
        const WorldBoxList<max_movement_obstacles>& blockers)
{
    bn::fixed_point own_foot_position = foot_position();
    if(! within_distance(own_foot_position, player_foot_position, disengage_distance))
    {
        reset_local_avoidance();
        _state = State::RETURN;
        _status_icon_timer = return_question_frames;
        _set_awareness_icon(StatusIcon::RETURN_QUESTION);
        return;
    }

    bn::fixed horizontal = player_hurtbox.center.x() - position().x();
    bn::fixed vertical = player_hurtbox.center.y() - position().y();
    switch(ranged_decision(own_foot_position, player_foot_position))
    {
    case RangedDecision::RETREAT:
    {
        Direction retreat_direction = direction_toward(player_foot_position, own_foot_position, direction());
        move_toward_with_local_avoidance(
                position() + directional_offset(retreat_direction, ranged_commit_radius),
                chase_speed, blockers);
        return;
    }
    case RangedDecision::HOLD:
        reset_local_avoidance();
        _start_attack(direction_from_components(sign(horizontal), sign(vertical), direction()));
        return;
    case RangedDecision::APPROACH:
        move_toward_with_local_avoidance(player_foot_position, chase_speed, blockers);
        return;
    default:
        return;
    }
}

void CrossbowGoblin::_update_telegraph(const WorldBox& player_hurtbox, CrossbowProjectilePool& projectiles)
{
    // Telegraph is a visible aim period. It never moves or rechecks Commit,
    // but its final shot tracks the target up to the launch frame.
#if defined(GAIN_PERF_DEBUG_LOGS)
    Direction previous_direction = _attack_direction;
#endif
    _attack_direction = direction_from_components(
            sign(player_hurtbox.center.x() - position().x()),
            sign(player_hurtbox.center.y() - position().y()), _attack_direction);
    _locked_target = player_hurtbox.center;
#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().crossbow_telegraph_frames;
    ++perf_stats().crossbow_telegraph_apply_movement_calls;
    perf_stats().crossbow_direction_changes += _attack_direction != previous_direction;
#endif
    apply_movement(position(), _attack_direction);
    _set_telegraph_visible(true);
    RangedAttackTick tick = ranged_attack_tick(_state, _state_timer);
    _state = tick.next_state;
    _state_timer = tick.next_timer;
    if(tick.spawn_projectile)
    {
        int horizontal; int vertical; direction_components(_attack_direction, horizontal, vertical);
        // actor_id() is a stable value identity, not an owner pointer.
        projectiles.spawn(actor_id(), { position().x() + horizontal * 6, position().y() + vertical * 6 },
                          _locked_target);
        _set_telegraph_visible(false); _status_icon_frame = 0;
        _set_recovery_hourglass_visible(true);
    }
}

void CrossbowGoblin::_update_return(
        const bn::fixed_point& player_foot_position, const WorldBoxList<max_movement_obstacles>& blockers)
{
    if(within_distance(foot_position(), player_foot_position, discovery_distance))
    { _state = State::CHASE; _status_icon_timer = discovery_flash_frames; _set_awareness_icon(StatusIcon::DISCOVERY_FLASH); return; }
    if(within_distance(position(), home_position(), 1))
    { _state = State::ROAM; _state_timer = roam_direction_frames; _status_icon_timer = 0; _set_telegraph_visible(false); return; }
    move_toward(home_position(), chase_speed, blockers);
}

void CrossbowGoblin::_start_attack(Direction direction)
{
    _attack_direction = direction; apply_movement(position(), direction);
    _status_icon_timer = 0; _state = State::TELEGRAPH; _state_timer = telegraph_frames; _set_telegraph_visible(true);
}

void CrossbowGoblin::_die()
{
    _status_icon_timer = 0; _set_telegraph_visible(false); _state = State::DEAD;
    defeat_enemy(respawn_delay_ticks);
}

void CrossbowGoblin::_set_telegraph_visible(bool visible)
{
    if(! visible) { _status_icon_sprite.set_visible(false); _status_icon = StatusIcon::NONE; return; }
    if(_status_icon != StatusIcon::TELEGRAPH) _show_status_icon(bn::sprite_items::enemy_telegraph, _telegraph_tiles, _telegraph_palette, StatusIcon::TELEGRAPH, 0);
    else
    {
#if defined(GAIN_PERF_DEBUG_LOGS)
        ++perf_stats().status_icon_position_updates;
#endif
        _status_icon_sprite.set_position(status_icon_position(position())); _status_icon_sprite.set_visible(true);
    }
}

void CrossbowGoblin::_set_recovery_hourglass_visible(bool visible)
{
    if(! visible) { _status_icon_sprite.set_visible(false); _status_icon = StatusIcon::NONE; return; }
    int frame = recovery_hourglass_frame(_state_timer);
    if(_status_icon != StatusIcon::RECOVERY_HOURGLASS || _status_icon_frame != frame)
        _show_status_icon(bn::sprite_items::goblin_recovery_hourglass, _recovery_hourglass_tiles[frame], _recovery_hourglass_palette, StatusIcon::RECOVERY_HOURGLASS, frame);
    else { _status_icon_sprite.set_position(status_icon_position(position())); _status_icon_sprite.set_visible(true); }
}

void CrossbowGoblin::_set_awareness_icon(StatusIcon icon)
{
    BN_ASSERT(icon == StatusIcon::DISCOVERY_FLASH || icon == StatusIcon::RETURN_QUESTION);
    int frame = icon == StatusIcon::DISCOVERY_FLASH ? discovery_bulb_frame(_status_icon_timer) : 2;
    if(_status_icon != icon || _status_icon_frame != frame)
        _show_status_icon(bn::sprite_items::goblin_awareness_icons, _awareness_icon_tiles[frame], _awareness_icon_palette, icon, frame);
    else { _status_icon_sprite.set_position(status_icon_position(position())); _status_icon_sprite.set_visible(true); }
}

void CrossbowGoblin::_show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                                       const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame)
{
    _status_icon_sprite.set_tiles_and_palette(item.shape_size(), tiles, palette);
    _status_icon_sprite.set_position(status_icon_position(position())); _status_icon_sprite.set_visible(true);
    _status_icon = icon; _status_icon_frame = frame;
}

void CrossbowGoblin::_update_timed_status_icon()
{
    if(_status_icon_timer == 0) return;
    if(--_status_icon_timer == 0) { _set_telegraph_visible(false); return; }
    _set_awareness_icon(_status_icon);
}
