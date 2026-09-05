#include "character/goblin.h"

#include "bn_array.h"
#include "bn_assert.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin.h"
#include "bn_sprite_items_goblin_awareness_icons.h"
#include "bn_sprite_items_goblin_recovery_hourglass.h"

#include "combat/collision/collision_math.h"
#include "combat/hit_effect_manager.h"
#include "combat/melee/swordsman_attack.h"

namespace
{
    constexpr CharacterDefinition goblin_definition = {
        CharacterId::GOBLIN, "GOBLIN", 6, 1
    };
    constexpr int player_target_id = 0;
    constexpr int home_radius = 28;
    constexpr int discovery_distance = 42;
    constexpr int disengage_distance = 58;
    constexpr int roam_direction_frames = 42;
    constexpr int telegraph_frames = 54;
    constexpr int active_frames = 8;
    constexpr int recovery_frames = 28;
    constexpr int discovery_flash_frames = 24;
    constexpr int discovery_bulb_white_frames = 10;
    constexpr int return_question_frames = 21;
    constexpr int respawn_delay_ticks = 120;
    // The former directional commit rectangles reached at most sqrt(19^2 + 19^2)
    // pixels between the Goblin and Samurai foot anchors. Round that up so the
    // initial 360-degree radius preserves the old maximum start distance.
    constexpr int melee_commit_radius = 27;
    constexpr int goblin_size = 16;

    static_assert(goblin_definition.id == CharacterId::GOBLIN);
    static_assert(goblin_definition.display_name_length == 6);
    static_assert(goblin_definition.max_hp == 1);
    [[nodiscard]] constexpr int discovery_bulb_frame(int remaining_frames)
    {
        return remaining_frames > discovery_flash_frames - discovery_bulb_white_frames ? 0 : 1;
    }

    [[nodiscard]] constexpr int recovery_hourglass_frame(int remaining_frames)
    {
        return remaining_frames > recovery_frames / 2 ? 0 : 1;
    }

    constexpr bn::fixed roam_speed = bn::fixed(1) / 4;
    constexpr bn::fixed chase_speed = bn::fixed(1) / 2;
    constexpr MovementBounds goblin_movement_bounds = Battlefield::movement_bounds(goblin_size, goblin_size);
    constexpr Pushbox goblin_body_pushbox = { { 0, 3, 6, 6 } };
    constexpr CollisionBody goblin_collision_body = {
        { { 0, 1, 8, 10 } },
        goblin_body_pushbox
    };

    static_assert(goblin_body_pushbox.box.offset_y == 3);
    static_assert(goblin_body_pushbox.box.width == 6 && goblin_body_pushbox.box.height == 6);
    constexpr bn::array<Direction, 4> roam_directions = {
        Direction::RIGHT, Direction::DOWN, Direction::LEFT, Direction::UP
    };
    [[nodiscard]] constexpr Hitbox hitbox(int x, int y, int width, int height)
    {
        return {{ x, y, width, height }};
    }

    constexpr bn::array<Hitbox, 8> attack_hitboxes = {
        hitbox(0, 14, 12, 10), hitbox(-10, 10, 12, 10),
        hitbox(-14, 0, 10, 12), hitbox(-10, -10, 12, 10),
        hitbox(0, -14, 12, 10), hitbox(10, -10, 12, 10),
        hitbox(14, 0, 10, 12), hitbox(10, 10, 12, 10)
    };

    [[nodiscard]] constexpr Direction direction_from_components(int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical)
        {
            return fallback;
        }

        if(vertical > 0)
        {
            return horizontal < 0 ? Direction::DOWN_LEFT : horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN;
        }

        if(vertical < 0)
        {
            return horizontal < 0 ? Direction::UP_LEFT : horizontal > 0 ? Direction::UP_RIGHT : Direction::UP;
        }

        return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
    }

    [[nodiscard]] constexpr int absolute(int value)
    {
        return value < 0 ? -value : value;
    }

    [[nodiscard]] constexpr Direction nearest_direction(int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical)
        {
            return fallback;
        }

        int horizontal_distance = absolute(horizontal);
        int vertical_distance = absolute(vertical);
        if(horizontal_distance * 12 < vertical_distance * 5)
        {
            return vertical < 0 ? Direction::UP : Direction::DOWN;
        }

        if(vertical_distance * 12 < horizontal_distance * 5)
        {
            return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
        }

        return direction_from_components(horizontal < 0 ? -1 : 1, vertical < 0 ? -1 : 1, fallback);
    }

    [[nodiscard]] Direction nearest_direction(const bn::fixed_point& origin, const bn::fixed_point& target,
                                              Direction fallback)
    {
        return nearest_direction((target.x() - origin.x()).integer(), (target.y() - origin.y()).integer(), fallback);
    }

    [[nodiscard]] constexpr bool attack_direction_tests()
    {
        return nearest_direction(20, 0, Direction::DOWN) == Direction::RIGHT &&
               nearest_direction(-20, 1, Direction::DOWN) == Direction::LEFT &&
               nearest_direction(1, -20, Direction::DOWN) == Direction::UP &&
               nearest_direction(0, 20, Direction::UP) == Direction::DOWN &&
               nearest_direction(20, 9, Direction::DOWN) == Direction::DOWN_RIGHT &&
               nearest_direction(-20, -9, Direction::DOWN) == Direction::UP_LEFT &&
               nearest_direction(20, 8, Direction::DOWN) == Direction::RIGHT &&
               nearest_direction(8, 20, Direction::DOWN) == Direction::DOWN &&
               attack_hitboxes[int(Direction::DOWN)].box.offset_y > 0 &&
               attack_hitboxes[int(Direction::DOWN_LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::DOWN_LEFT)].box.offset_y > 0 &&
               attack_hitboxes[int(Direction::LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::UP_LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::UP_LEFT)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::UP)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::UP_RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::UP_RIGHT)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::DOWN_RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::DOWN_RIGHT)].box.offset_y > 0;
    }

    static_assert(attack_direction_tests());
    static_assert(Enemy::within_distance({ 0, 0 }, { discovery_distance, 0 }, discovery_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { discovery_distance + 1, 0 }, discovery_distance));
    static_assert(Enemy::within_distance({ 0, 0 }, { 29, 29 }, discovery_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { 30, 30 }, discovery_distance));
    static_assert(Enemy::within_distance({ 0, 0 }, { disengage_distance, 0 }, disengage_distance));
    static_assert(! Enemy::within_distance({ 0, 0 }, { disengage_distance + 1, 0 }, disengage_distance));
    static_assert(26 * 26 < (19 * 19) + (19 * 19));
    static_assert(melee_commit_radius * melee_commit_radius >= (19 * 19) + (19 * 19));
    static_assert(Enemy::within_distance({ 0, 0 }, { melee_commit_radius, 0 }, melee_commit_radius));
    static_assert(Enemy::within_distance({ 0, 0 }, { -melee_commit_radius, 0 }, melee_commit_radius));
    static_assert(! Enemy::within_distance({ 0, 0 }, { melee_commit_radius + 1, 0 }, melee_commit_radius));
    static_assert(! Enemy::within_distance({ 0, 0 }, { 20, 20 }, melee_commit_radius));
    static_assert(world_foot_position({ 0, 0 }, goblin_body_pushbox) == bn::fixed_point(0, 6));

    [[nodiscard]] WorldBox attack_hitbox(const bn::fixed_point& position, Direction direction)
    {
        return world_box(position, attack_hitboxes[int(direction)].box);
    }

    static_assert(discovery_bulb_frame(24) == 0);
    static_assert(discovery_bulb_frame(15) == 0);
    static_assert(discovery_bulb_frame(14) == 1);
    static_assert(discovery_bulb_frame(1) == 1);
    static_assert(recovery_hourglass_frame(28) == 0);
    static_assert(recovery_hourglass_frame(15) == 0);
    static_assert(recovery_hourglass_frame(14) == 1);
    static_assert(recovery_hourglass_frame(1) == 1);

    [[nodiscard]] bn::fixed_point status_icon_position(const bn::fixed_point& position)
    {
        bn::fixed icon_y = position.y() - 13;
        if(icon_y < -70)
        {
            icon_y = -70;
        }
        return { position.x(), icon_y };
    }
}

Goblin::Goblin(const bn::fixed_point& home_position, int target_id) :
    Enemy(bn::sprite_items::goblin, home_position, Direction::DOWN, chase_speed, goblin_collision_body,
          goblin_definition, goblin_movement_bounds, target_id),
    _awareness_icon_tiles{{
        bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(0),
        bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(1),
        bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(2)
    }},
    _awareness_icon_palette(bn::sprite_items::goblin_awareness_icons.palette_item().create_palette()),
    _telegraph_tiles(bn::sprite_items::enemy_telegraph.tiles_item().create_tiles()),
    _telegraph_palette(bn::sprite_items::enemy_telegraph.palette_item().create_palette()),
    _recovery_hourglass_tiles{{
        bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(0),
        bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(1)
    }},
    _recovery_hourglass_palette(bn::sprite_items::goblin_recovery_hourglass.palette_item().create_palette()),
    _status_icon_sprite(bn::sprite_items::goblin_awareness_icons.create_sprite(home_position, 0))
{
    _status_icon_sprite.set_z_order(-2);
    _status_icon_sprite.set_visible(false);
}

void Goblin::enter()
{
    enter_enemy();
    _state = State::ROAM;
    _state_timer = roam_direction_frames;
    _roam_direction_index = 0;
    _attack_direction = Direction::DOWN;
    _attack_hit_registry.reset();
    _status_icon_frame = 0;
    _status_icon_timer = 0;
    reset_local_avoidance();
    _status_icon = StatusIcon::NONE;
    _set_telegraph_visible(false);
}

void Goblin::deactivate()
{
    deactivate_enemy();
    _state = State::DEAD;
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
}

void Goblin::hide()
{
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
    hide_enemy();
}

void Goblin::update(const bn::fixed_point& player_foot_position, bool player_on_same_layer,
                    const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(! active())
    {
        if(respawn_position_ready(blocking_pushboxes))
        {
            enter();
        }
        return;
    }

    _update_timed_status_icon();

    // A floor transition does not exist yet.  An enemy on the other logical
    // layer therefore keeps its local roaming behavior instead of chasing a
    // player it cannot physically reach.
    if(! player_on_same_layer && (_state == State::CHASE || _state == State::RETURN ||
                                  _state == State::TELEGRAPH || _state == State::ACTIVE ||
                                  _state == State::RECOVERY))
    {
        reset_local_avoidance();
        _set_telegraph_visible(false);
        _set_recovery_hourglass_visible(false);
        _state = State::ROAM;
        _state_timer = roam_direction_frames;
        _status_icon_timer = 0;
        _set_awareness_icon(StatusIcon::NONE);
    }

    switch(_state)
    {
    case State::ROAM:
        if(player_on_same_layer && within_distance(foot_position(), player_foot_position, discovery_distance))
        {
            _state = State::CHASE;
            _status_icon_timer = discovery_flash_frames;
            _set_awareness_icon(StatusIcon::DISCOVERY_FLASH);
        }
        else
        {
            _update_roam(blocking_pushboxes);
        }
        break;
    case State::CHASE:
        _update_chase(player_foot_position, blocking_pushboxes);
        break;
    case State::TELEGRAPH:
        _update_telegraph();
        break;
    case State::ACTIVE:
        _update_active();
        break;
    case State::RECOVERY:
        _update_recovery();
        break;
    case State::RETURN:
        _update_return(player_foot_position, blocking_pushboxes);
        break;
    case State::DEAD:
        break;
    default:
        break;
    }

    if(_status_icon != StatusIcon::NONE)
    {
        _status_icon_sprite.set_position(status_icon_position(position()));
    }
}

void Goblin::resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects)
{
    if(! active())
    {
        return;
    }

    int damage = attack.try_hit(target_id(), position(), collision_body().hurtbox);
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

int Goblin::resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                               HitEffectManager& hit_effects)
{
    if(! attack_active() || _attack_hit_registry.contains(player_target_id))
    {
        return 0;
    }

    WorldBox hitbox = attack_hitbox(position(), _attack_direction);
    WorldBox hurtbox = world_box(player_position, player_hurtbox.box);
    if(touches_or_intersects(hitbox, hurtbox) && _attack_hit_registry.add(player_target_id))
    {
        hit_effects.spawn(hurtbox.center);
        return 1;
    }

    return 0;
}

bool Goblin::attack_active() const
{
    return active() && _state == State::ACTIVE;
}

Goblin::State Goblin::state() const
{
    return _state;
}

void Goblin::append_debug_shapes(
        CollisionDebugBoxList& boxes, CollisionDebugRadiusList& radii) const
{
    if(! active())
    {
        return;
    }

    boxes.add(world_hurtbox(), CollisionDebugBoxType::HURTBOX);
    boxes.add(world_pushbox(), CollisionDebugBoxType::PUSHBOX);
    if(attack_active())
    {
        boxes.add(attack_hitbox(position(), _attack_direction), CollisionDebugBoxType::HITBOX);
    }

    bn::fixed_point center = foot_position();
    radii.add(center, discovery_distance, CollisionDebugRadiusType::DISCOVERY);
    radii.add(center, disengage_distance, CollisionDebugRadiusType::DISENGAGE);
    radii.add(center, melee_commit_radius, CollisionDebugRadiusType::MELEE_COMMIT);
}

void Goblin::_update_roam(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    move_direction(roam_directions[_roam_direction_index], roam_speed, blocking_pushboxes, true, home_radius);
    --_state_timer;
    if(_state_timer == 0)
    {
        _roam_direction_index = (_roam_direction_index + 1) % roam_directions.size();
        _state_timer = roam_direction_frames;
    }
}

void Goblin::_update_chase(const bn::fixed_point& player_foot_position,
                           const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    bn::fixed_point own_foot_position = foot_position();
    if(! within_distance(own_foot_position, player_foot_position, disengage_distance) && ! local_detour_active())
    {
        reset_local_avoidance();
        _state = State::RETURN;
        _status_icon_timer = return_question_frames;
        _set_awareness_icon(StatusIcon::RETURN_QUESTION);
        return;
    }

    Direction attack_direction = nearest_direction(own_foot_position, player_foot_position, direction());
    if(within_distance(own_foot_position, player_foot_position, melee_commit_radius))
    {
        _start_attack(attack_direction);
        return;
    }

    move_toward_with_local_avoidance(player_foot_position, chase_speed, blocking_pushboxes);
}

void Goblin::_update_telegraph()
{
    _set_telegraph_visible(true);
    if(--_state_timer == 0)
    {
        _set_telegraph_visible(false);
        _state = State::ACTIVE;
        _state_timer = active_frames;
    }
}

void Goblin::_update_active()
{
    if(--_state_timer == 0)
    {
        _finish_attack();
    }
}

void Goblin::_update_recovery()
{
    _set_recovery_hourglass_visible(true);

    if(--_state_timer == 0)
    {
        _set_recovery_hourglass_visible(false);
        _state = State::CHASE;
    }
}

void Goblin::_update_return(const bn::fixed_point& player_foot_position,
                            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(within_distance(foot_position(), player_foot_position, discovery_distance))
    {
        _state = State::CHASE;
        _status_icon_timer = discovery_flash_frames;
        _set_awareness_icon(StatusIcon::DISCOVERY_FLASH);
        return;
    }

    if(within_distance(position(), home_position(), 1))
    {
        _state = State::ROAM;
        _state_timer = roam_direction_frames;
        _status_icon_timer = 0;
        _set_telegraph_visible(false);
        return;
    }

    move_toward(home_position(), chase_speed, blocking_pushboxes);
}

void Goblin::_start_attack(Direction direction)
{
    reset_local_avoidance();
    _attack_direction = direction;
    apply_movement(position(), _attack_direction);
    _attack_hit_registry.reset();
    _status_icon_timer = 0;
    _state = State::TELEGRAPH;
    _state_timer = telegraph_frames;
    _set_telegraph_visible(true);
}

void Goblin::_finish_attack()
{
    _attack_hit_registry.reset();
    _state = State::RECOVERY;
    _state_timer = recovery_frames;
    _status_icon_frame = 0;
    _status_icon_timer = 0;
    _set_recovery_hourglass_visible(true);
}

void Goblin::_die()
{
    if(! active())
    {
        return;
    }

    _attack_hit_registry.reset();
    _status_icon_timer = 0;
    _set_telegraph_visible(false);
    _state = State::DEAD;
    _state_timer = 0;
    defeat_enemy(respawn_delay_ticks);
}

void Goblin::_set_telegraph_visible(bool visible)
{
    if(! visible)
    {
        _status_icon_sprite.set_visible(false);
        _status_icon = StatusIcon::NONE;
        return;
    }

    bn::fixed_point telegraph_position = status_icon_position(position());
    if(_status_icon != StatusIcon::TELEGRAPH)
    {
        _show_status_icon(bn::sprite_items::enemy_telegraph, _telegraph_tiles, _telegraph_palette,
                          StatusIcon::TELEGRAPH, 0, telegraph_position);
    }
    else
    {
        _status_icon_sprite.set_position(telegraph_position);
        _status_icon_sprite.set_visible(true);
    }
}

void Goblin::_set_recovery_hourglass_visible(bool visible)
{
    if(! visible)
    {
        _status_icon_sprite.set_visible(false);
        _status_icon = StatusIcon::NONE;
        return;
    }

    int frame = recovery_hourglass_frame(_state_timer);
    bn::fixed_point hourglass_position = status_icon_position(position());
    if(_status_icon != StatusIcon::RECOVERY_HOURGLASS || _status_icon_frame != frame)
    {
        _show_status_icon(bn::sprite_items::goblin_recovery_hourglass, _recovery_hourglass_tiles[frame],
                          _recovery_hourglass_palette, StatusIcon::RECOVERY_HOURGLASS, frame, hourglass_position);
    }
    else
    {
        _status_icon_sprite.set_position(hourglass_position);
        _status_icon_sprite.set_visible(true);
    }
}

void Goblin::_set_awareness_icon(StatusIcon icon)
{
    BN_ASSERT(icon == StatusIcon::DISCOVERY_FLASH || icon == StatusIcon::RETURN_QUESTION);

    int frame = 2;
    if(icon == StatusIcon::DISCOVERY_FLASH)
    {
        frame = discovery_bulb_frame(_status_icon_timer);
    }
    bn::fixed_point icon_position = status_icon_position(position());
    if(_status_icon != icon || _status_icon_frame != frame)
    {
        _show_status_icon(bn::sprite_items::goblin_awareness_icons, _awareness_icon_tiles[frame],
                          _awareness_icon_palette, icon, frame, icon_position);
    }
    else
    {
        _status_icon_sprite.set_position(icon_position);
        _status_icon_sprite.set_visible(true);
    }
}

void Goblin::_show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                               const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame,
                               const bn::fixed_point& icon_position)
{
    _status_icon_sprite.set_tiles_and_palette(item.shape_size(), tiles, palette);
    _status_icon_sprite.set_position(icon_position);
    _status_icon_sprite.set_visible(true);
    _status_icon = icon;
    _status_icon_frame = frame;
}

void Goblin::_update_timed_status_icon()
{
    if(_status_icon_timer == 0)
    {
        return;
    }

    --_status_icon_timer;
    if(_status_icon_timer == 0)
    {
        _set_telegraph_visible(false);
        return;
    }

    _set_awareness_icon(_status_icon);
}
