#include "character/crossbow_goblin.h"

#include "bn_assert.h"
#include "bn_sprite_items_crossbow_goblin.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin_awareness_icons.h"
#include "bn_sprite_items_goblin_recovery_hourglass.h"

#include "combat/collision/collision_math.h"
#include "combat/collision/movement_collision.h"
#include "combat/crossbow_projectile_pool.h"
#include "combat/hit_effect_manager.h"
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
    constexpr int respawn_clearance = 2;
    constexpr int goblin_size = 16;
    constexpr int minimum_attack_distance = 64;
    constexpr int maximum_attack_distance = 80;
    constexpr int commit_distance = 72;

    static_assert(crossbow_goblin_definition.id == CharacterId::CROSSBOW_GOBLIN);
    static_assert(crossbow_goblin_definition.display_name_length == 15);
    static_assert(crossbow_goblin_definition.max_hp == 1);
    constexpr int commit_cell_size = goblin_size;
    constexpr int commit_columns = 3;
    constexpr int commit_rows = 2;
    constexpr int commit_box_count = commit_columns * commit_rows;
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

    [[nodiscard]] bool within_distance(const bn::fixed_point& first, const bn::fixed_point& second, int distance)
    {
        bn::fixed x = first.x() - second.x();
        bn::fixed y = first.y() - second.y();
        return (x * x) + (y * y) <= distance * distance;
    }

    [[nodiscard]] int sign(bn::fixed value) { return value > 0 ? 1 : value < 0 ? -1 : 0; }

    [[nodiscard]] Direction direction_from_components(int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical) return fallback;
        if(vertical > 0) return horizontal < 0 ? Direction::DOWN_LEFT : horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN;
        if(vertical < 0) return horizontal < 0 ? Direction::UP_LEFT : horizontal > 0 ? Direction::UP_RIGHT : Direction::UP;
        return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
    }

    void direction_components(Direction direction, int& horizontal, int& vertical)
    {
        static constexpr bn::array<int, 8> horizontal_components = { 0, -1, -1, -1, 0, 1, 1, 1 };
        static constexpr bn::array<int, 8> vertical_components = { 1, 1, 0, -1, -1, -1, 0, 1 };
        horizontal = horizontal_components[int(direction)];
        vertical = vertical_components[int(direction)];
    }

    [[nodiscard]] bn::fixed clamp(bn::fixed value, bn::fixed minimum, bn::fixed maximum)
    {
        return value < minimum ? minimum : value > maximum ? maximum : value;
    }

    [[nodiscard]] bn::fixed_point directional_offset(Direction direction, int distance)
    {
        int horizontal;
        int vertical;
        direction_components(direction, horizontal, vertical);
        bn::fixed step = (horizontal && vertical) ? bn::fixed(distance) * diagonal_ratio : distance;
        return { horizontal * step, vertical * step };
    }

    [[nodiscard]] bn::array<WorldBox, commit_box_count> commit_boxes(
            const bn::fixed_point& position, Direction direction)
    {
        bn::array<WorldBox, commit_box_count> result;
        bn::fixed_point center = position + directional_offset(direction, commit_distance);
        int horizontal;
        int vertical;
        direction_components(direction, horizontal, vertical);
        Direction side_direction = direction_from_components(-vertical, horizontal, Direction::RIGHT);

        int index = 0;
        for(int forward_row = 0; forward_row < commit_rows; ++forward_row)
        {
            for(int lateral_column = -1; lateral_column <= 1; ++lateral_column)
            {
                bn::fixed_point cell_center = center + directional_offset(direction, forward_row * commit_cell_size) +
                                              directional_offset(side_direction, lateral_column * commit_cell_size);
                result[index] = { cell_center, commit_cell_size, commit_cell_size };
                ++index;
            }
        }
        return result;
    }

    [[nodiscard]] bool touches_commit_boxes(const bn::array<WorldBox, commit_box_count>& boxes,
                                             const WorldBox& player_hurtbox)
    {
        for(const WorldBox& box : boxes)
        {
            if(touches_or_intersects(box, player_hurtbox))
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bn::fixed_point attack_position(const bn::fixed_point& target, Direction direction)
    {
        return target - directional_offset(direction, commit_distance);
    }

    [[nodiscard]] Direction nearest_attack_direction(const bn::fixed_point& position,
                                                      const bn::fixed_point& target)
    {
        Direction result = Direction::DOWN;
        bn::fixed_point first_candidate = attack_position(target, result);
        bn::fixed best_distance = (first_candidate.x() - position.x()) * (first_candidate.x() - position.x()) +
                                  (first_candidate.y() - position.y()) * (first_candidate.y() - position.y());
        for(int index = 1; index < 8; ++index)
        {
            Direction candidate_direction = Direction(index);
            bn::fixed_point candidate = attack_position(target, candidate_direction);
            bn::fixed x = candidate.x() - position.x();
            bn::fixed y = candidate.y() - position.y();
            bn::fixed distance = (x * x) + (y * y);
            if(distance < best_distance)
            {
                result = candidate_direction;
                best_distance = distance;
            }
        }
        return result;
    }

    [[nodiscard]] constexpr WorldBox expanded_box(const WorldBox& box, int clearance)
    {
        return { box.center, box.width + clearance * 2, box.height + clearance * 2 };
    }

    [[nodiscard]] constexpr int discovery_bulb_frame(int remaining)
    {
        return remaining > discovery_flash_frames - discovery_bulb_white_frames ? 0 : 1;
    }

    [[nodiscard]] constexpr int recovery_hourglass_frame(int remaining) { return remaining > recovery_frames / 2 ? 0 : 1; }

    [[nodiscard]] constexpr bool attack_distance_allowed(int distance)
    {
        return distance >= minimum_attack_distance && distance <= maximum_attack_distance;
    }

    [[nodiscard]] bn::fixed_point status_icon_position(const bn::fixed_point& position)
    {
        bn::fixed y = position.y() - 13;
        return { position.x(), y < -70 ? -70 : y };
    }

    static_assert(! attack_distance_allowed(48));
    static_assert(! attack_distance_allowed(63));
    static_assert(attack_distance_allowed(64));
    static_assert(attack_distance_allowed(72));
    static_assert(attack_distance_allowed(80));
    static_assert(! attack_distance_allowed(81));
    static_assert(commit_box_count == 6);
}

CrossbowGoblin::CrossbowGoblin(const bn::fixed_point& home_position, int target_id) :
    Character(bn::sprite_items::crossbow_goblin, home_position, Direction::DOWN, chase_speed, crossbow_collision_body,
              crossbow_goblin_definition),
    _home_position(home_position), _locked_target(home_position),
    _awareness_icon_tiles{{ bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(0),
                             bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(1),
                             bn::sprite_items::goblin_awareness_icons.tiles_item().create_tiles(2) }},
    _awareness_icon_palette(bn::sprite_items::goblin_awareness_icons.palette_item().create_palette()),
    _telegraph_tiles(bn::sprite_items::enemy_telegraph.tiles_item().create_tiles()),
    _telegraph_palette(bn::sprite_items::enemy_telegraph.palette_item().create_palette()),
    _recovery_hourglass_tiles{{ bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(0),
                                 bn::sprite_items::goblin_recovery_hourglass.tiles_item().create_tiles(1) }},
    _recovery_hourglass_palette(bn::sprite_items::goblin_recovery_hourglass.palette_item().create_palette()),
    _status_icon_sprite(bn::sprite_items::goblin_awareness_icons.create_sprite(home_position, 0)), _target_id(target_id)
{
    _status_icon_sprite.set_z_order(-2);
    _status_icon_sprite.set_visible(false);
}

void CrossbowGoblin::enter()
{
    reset_health();
    _state = State::ROAM; _state_timer = roam_direction_frames; _roam_direction_index = 0;
    _attack_direction = Direction::DOWN; _alignment_direction = Direction::DOWN; _locked_target = _home_position;
    _status_icon = StatusIcon::NONE;
    _status_icon_timer = 0; _status_icon_frame = 0; _respawn_timer = 0; _respawning = false; _active = true;
    _set_telegraph_visible(false); apply_movement(_home_position, Direction::DOWN); set_visible(true);
}

void CrossbowGoblin::deactivate()
{
    _active = false;
    _respawning = false;
    _respawn_timer = 0;
    _state = State::DEAD;
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
    set_visible(false);
}

void CrossbowGoblin::hide()
{
    _set_telegraph_visible(false);
    _set_recovery_hourglass_visible(false);
    _status_icon = StatusIcon::NONE;
    _status_icon_sprite.set_visible(false);
    set_visible(false);
}

void CrossbowGoblin::set_home_position(const bn::fixed_point& position)
{
    _home_position = position;
    _locked_target = position;
}

void CrossbowGoblin::set_respawn_enabled(bool enabled)
{
    _respawn_enabled = enabled;
    if(! _respawn_enabled)
    {
        _respawning = false;
        _respawn_timer = 0;
    }
}

void CrossbowGoblin::update(const WorldBox& player_hurtbox, const WorldBox& player_pushbox,
                             const WorldBoxList<max_movement_obstacles>& blockers, CrossbowProjectilePool& projectiles)
{
    if(! _active) { if(_respawn_enabled) _update_respawn(blockers); return; }
    _update_timed_status_icon();
    switch(_state)
    {
    case State::ROAM:
        if(within_distance(position(), player_pushbox.center, discovery_distance))
        { _state = State::CHASE; _status_icon_timer = discovery_flash_frames; _set_awareness_icon(StatusIcon::DISCOVERY_FLASH); }
        else _update_roam(blockers);
        break;
    case State::CHASE: _update_chase(player_hurtbox, blockers); break;
    case State::TELEGRAPH: _update_telegraph(player_hurtbox, projectiles); break;
    case State::RECOVERY:
        _set_recovery_hourglass_visible(true);
        if(--_state_timer == 0) { _set_recovery_hourglass_visible(false); _state = State::CHASE; }
        break;
    case State::RETURN: _update_return(player_pushbox, blockers); break;
    case State::DEAD: break;
    default: break;
    }
    if(_status_icon != StatusIcon::NONE) _status_icon_sprite.set_position(status_icon_position(position()));
}

void CrossbowGoblin::resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects)
{
    int damage = _active ? attack.try_hit(_target_id, position(), collision_body().hurtbox) : 0;
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

bool CrossbowGoblin::active() const { return _active; }
WorldBox CrossbowGoblin::world_hurtbox() const { return world_box(position(), collision_body().hurtbox.box); }
WorldBox CrossbowGoblin::world_pushbox() const { return world_box(position(), collision_body().pushbox.box); }
WorldBox CrossbowGoblin::movement_obstacle_query_area() const
{
    return _active ? world_pushbox() : expanded_box(world_box(_home_position, collision_body().pushbox.box), respawn_clearance);
}

void CrossbowGoblin::append_collision_debug_boxes(const WorldBox&, CollisionDebugBoxList& boxes,
                                                   bool include_commit_boxes) const
{
    if(! _active) return;
    boxes.add(world_hurtbox(), CollisionDebugBoxType::HURTBOX); boxes.add(world_pushbox(), CollisionDebugBoxType::PUSHBOX);
    if(include_commit_boxes && (_state == State::CHASE || _state == State::TELEGRAPH))
    {
        Direction commit_direction = _state == State::TELEGRAPH ? _attack_direction : _alignment_direction;
        for(const WorldBox& box : commit_boxes(position(), commit_direction))
        {
            boxes.add(box, CollisionDebugBoxType::COMMIT_BOX);
        }
    }
}

void CrossbowGoblin::_update_roam(const WorldBoxList<max_movement_obstacles>& blockers)
{
    _move_direction(roam_directions[_roam_direction_index], roam_speed, blockers, true);
    if(--_state_timer == 0) { _roam_direction_index = (_roam_direction_index + 1) % roam_directions.size(); _state_timer = roam_direction_frames; }
}

void CrossbowGoblin::_update_chase(const WorldBox& player_hurtbox, const WorldBoxList<max_movement_obstacles>& blockers)
{
    if(! within_distance(position(), player_hurtbox.center, disengage_distance))
    { _state = State::RETURN; _status_icon_timer = return_question_frames; _set_awareness_icon(StatusIcon::RETURN_QUESTION); return; }
    _alignment_direction = nearest_attack_direction(position(), player_hurtbox.center);
    if(! within_distance(position(), player_hurtbox.center, minimum_attack_distance - 1) &&
       within_distance(position(), player_hurtbox.center, maximum_attack_distance) &&
       touches_commit_boxes(commit_boxes(position(), _alignment_direction), player_hurtbox))
    {
        _start_attack(_alignment_direction);
        return;
    }

    _move_toward(attack_position(player_hurtbox.center, _alignment_direction), blockers);
}

void CrossbowGoblin::_update_telegraph(const WorldBox& player_hurtbox, CrossbowProjectilePool& projectiles)
{
    // Telegraph is a visible aim period. It never moves or rechecks Commit,
    // but its final shot tracks the target up to the launch frame.
    _attack_direction = direction_from_components(sign(player_hurtbox.center.x() - position().x()),
                                                  sign(player_hurtbox.center.y() - position().y()), _attack_direction);
    _locked_target = player_hurtbox.center;
    apply_movement(position(), _attack_direction);
    _set_telegraph_visible(true);
    if(--_state_timer == 0)
    {
        int horizontal; int vertical; direction_components(_attack_direction, horizontal, vertical);
        projectiles.spawn({ position().x() + horizontal * 6, position().y() + vertical * 6 }, _locked_target);
        _set_telegraph_visible(false); _state = State::RECOVERY; _state_timer = recovery_frames; _status_icon_frame = 0;
        _set_recovery_hourglass_visible(true);
    }
}

void CrossbowGoblin::_update_return(const WorldBox& player_pushbox, const WorldBoxList<max_movement_obstacles>& blockers)
{
    if(within_distance(position(), player_pushbox.center, discovery_distance))
    { _state = State::CHASE; _status_icon_timer = discovery_flash_frames; _set_awareness_icon(StatusIcon::DISCOVERY_FLASH); return; }
    if(within_distance(position(), _home_position, 1))
    { _state = State::ROAM; _state_timer = roam_direction_frames; _status_icon_timer = 0; _set_telegraph_visible(false); return; }
    _move_toward(_home_position, blockers);
}

void CrossbowGoblin::_start_attack(Direction direction)
{
    _attack_direction = direction; apply_movement(position(), direction);
    _status_icon_timer = 0; _state = State::TELEGRAPH; _state_timer = telegraph_frames; _set_telegraph_visible(true);
}

void CrossbowGoblin::_die()
{
    _status_icon_timer = 0; _set_telegraph_visible(false); _state = State::DEAD;
    _respawn_timer = _respawn_enabled ? respawn_delay_ticks : 0;
    _respawning = _respawn_enabled; _active = false; set_visible(false);
}

void CrossbowGoblin::_update_respawn(const WorldBoxList<max_movement_obstacles>& blockers)
{
    if(! _respawning) return;
    if(_respawn_timer > 0) { --_respawn_timer; return; }
    if(_respawn_position_is_safe(blockers)) enter();
}

bool CrossbowGoblin::_respawn_position_is_safe(const WorldBoxList<max_movement_obstacles>& blockers) const
{
    WorldBox safe = expanded_box(world_box(_home_position, collision_body().pushbox.box), respawn_clearance);
    for(int index = 0; index < blockers.count; ++index) if(touches_or_intersects(safe, blockers.boxes[index])) return false;
    return true;
}

void CrossbowGoblin::_move_direction(Direction movement_direction, bn::fixed speed,
                                     const WorldBoxList<max_movement_obstacles>& blockers, bool constrain_to_home)
{
    int horizontal; int vertical; direction_components(movement_direction, horizontal, vertical);
    bn::fixed step = (horizontal && vertical) ? speed * diagonal_ratio : speed;
    bn::fixed_point next(position().x() + horizontal * step, position().y() + vertical * step);
    if(constrain_to_home)
    { next.set_x(clamp(next.x(), _home_position.x() - home_radius, _home_position.x() + home_radius)); next.set_y(clamp(next.y(), _home_position.y() - home_radius, _home_position.y() + home_radius)); }
    apply_movement(resolve_movement(position(), next - position(), collision_body().pushbox, blockers, movement_bounds), movement_direction);
}

void CrossbowGoblin::_move_toward(const bn::fixed_point& target, const WorldBoxList<max_movement_obstacles>& blockers)
{
    _move_direction(direction_from_components(sign(target.x() - position().x()), sign(target.y() - position().y()), direction()), chase_speed, blockers, false);
}

void CrossbowGoblin::_set_telegraph_visible(bool visible)
{
    if(! visible) { _status_icon_sprite.set_visible(false); _status_icon = StatusIcon::NONE; return; }
    if(_status_icon != StatusIcon::TELEGRAPH) _show_status_icon(bn::sprite_items::enemy_telegraph, _telegraph_tiles, _telegraph_palette, StatusIcon::TELEGRAPH, 0);
    else { _status_icon_sprite.set_position(status_icon_position(position())); _status_icon_sprite.set_visible(true); }
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
