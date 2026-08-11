#include "character/goblin.h"

#include "bn_array.h"
#include "bn_assert.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin.h"
#include "bn_sprite_items_goblin_awareness_icons.h"
#include "bn_sprite_items_goblin_recovery_hourglass.h"

#include "combat/collision/collision_math.h"
#include "combat/collision/movement_collision.h"
#include "combat/hit_effect_manager.h"
#include "combat/melee/swordsman_attack.h"

namespace
{
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
    constexpr int respawn_clearance = 2;
    constexpr int commit_margin = 2;
    constexpr int goblin_size = 16;
    constexpr int stuck_frames_before_detour = 6;
    // At 0.5px per frame, this moves a goblin one 16px Stage obstacle width.
    constexpr int max_detour_frames = 32;

    [[nodiscard]] constexpr int next_stuck_frames(int current_frames, bool direct_move_full)
    {
        if(direct_move_full)
        {
            return 0;
        }

        return current_frames < stuck_frames_before_detour ? current_frames + 1 : current_frames;
    }

    [[nodiscard]] constexpr int next_detour_frames(int current_frames)
    {
        return current_frames > 0 ? current_frames - 1 : 0;
    }

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
    constexpr bn::fixed diagonal_ratio(0.70710678f);
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

    [[nodiscard]] bool within_distance(const bn::fixed_point& first, const bn::fixed_point& second,
                                       int distance)
    {
        bn::fixed delta_x = first.x() - second.x();
        bn::fixed delta_y = first.y() - second.y();
        return (delta_x * delta_x) + (delta_y * delta_y) <= distance * distance;
    }

    [[nodiscard]] int sign(bn::fixed value)
    {
        return value > 0 ? 1 : value < 0 ? -1 : 0;
    }

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

    [[nodiscard]] bn::fixed squared_distance(const bn::fixed_point& first, const bn::fixed_point& second)
    {
        bn::fixed x = first.x() - second.x();
        bn::fixed y = first.y() - second.y();
        return (x * x) + (y * y);
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

    void direction_components(Direction direction, int& horizontal, int& vertical)
    {
        static constexpr bn::array<int, 8> horizontal_components = { 0, -1, -1, -1, 0, 1, 1, 1 };
        static constexpr bn::array<int, 8> vertical_components = { 1, 1, 0, -1, -1, -1, 0, 1 };
        horizontal = horizontal_components[int(direction)];
        vertical = vertical_components[int(direction)];
    }

    [[nodiscard]] constexpr Direction offset_direction(Direction direction, int offset)
    {
        int index = (int(direction) + offset) % 8;
        if(index < 0)
        {
            index += 8;
        }
        return Direction(index);
    }

    [[nodiscard]] constexpr bn::array<Direction, 4> detour_candidates(
            Direction desired_direction, bool clockwise_first)
    {
        int preferred_offset = clockwise_first ? 1 : -1;
        int tangential_offset = int(desired_direction) % 2 == 0 ? 2 : 1;
        return {
            offset_direction(desired_direction, preferred_offset * tangential_offset),
            offset_direction(desired_direction, -preferred_offset * tangential_offset),
            offset_direction(desired_direction, preferred_offset * (tangential_offset + 1)),
            offset_direction(desired_direction, -preferred_offset * (tangential_offset + 1))
        };
    }

    [[nodiscard]] bn::fixed clamp(bn::fixed value, bn::fixed minimum, bn::fixed maximum)
    {
        return value < minimum ? minimum : value > maximum ? maximum : value;
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
    static_assert(detour_candidates(Direction::RIGHT, false)[0] == Direction::UP);
    static_assert(detour_candidates(Direction::RIGHT, false)[1] == Direction::DOWN);
    static_assert(detour_candidates(Direction::UP_RIGHT, false)[0] == Direction::UP);
    static_assert(detour_candidates(Direction::UP_RIGHT, false)[1] == Direction::RIGHT);
    static_assert(detour_candidates(Direction::RIGHT, true)[0] == Direction::DOWN);
    static_assert(next_stuck_frames(0, false) == 1);
    static_assert(next_stuck_frames(stuck_frames_before_detour - 1, false) == stuck_frames_before_detour);
    static_assert(next_stuck_frames(stuck_frames_before_detour, false) == stuck_frames_before_detour);
    static_assert(next_stuck_frames(stuck_frames_before_detour, true) == 0);
    static_assert(next_detour_frames(max_detour_frames) == max_detour_frames - 1);
    static_assert(next_detour_frames(1) == 0);
    static_assert(next_detour_frames(0) == 0);

    [[nodiscard]] WorldBox attack_hitbox(const bn::fixed_point& position, Direction direction)
    {
        return world_box(position, attack_hitboxes[int(direction)].box);
    }

    [[nodiscard]] constexpr WorldBox commit_box(const WorldBox& hitbox)
    {
        return { hitbox.center, hitbox.width - (commit_margin * 2), hitbox.height - (commit_margin * 2) };
    }

    [[nodiscard]] constexpr WorldBox expanded_box(const WorldBox& box, int clearance)
    {
        return { box.center, box.width + (clearance * 2), box.height + (clearance * 2) };
    }

    static_assert(commit_box({ { 0, 0 }, 12, 10 }).width == 8);
    static_assert(commit_box({ { 0, 0 }, 12, 10 }).height == 6);
    static_assert(! touches_or_intersects(commit_box({ { 0, 0 }, 12, 10 }), { { 10, 0 }, 10, 10 }));
    static_assert(touches_or_intersects(commit_box({ { 0, 0 }, 12, 10 }), { { 9, 0 }, 10, 10 }));
    static_assert(expanded_box({ { 0, 0 }, 6, 6 }, respawn_clearance).width == 10);
    static_assert(expanded_box({ { 0, 0 }, 6, 6 }, respawn_clearance).height == 10);
    static_assert(touches_or_intersects(expanded_box({ { 0, 0 }, 6, 6 }, respawn_clearance), { { 9, 0 }, 8, 8 }));
    static_assert(! touches_or_intersects(expanded_box({ { 0, 0 }, 6, 6 }, respawn_clearance), { { 10, 0 }, 8, 8 }));
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
    Character(bn::sprite_items::goblin, home_position, Direction::DOWN, chase_speed, goblin_collision_body),
    _home_position(home_position),
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
    _status_icon_sprite(bn::sprite_items::goblin_awareness_icons.create_sprite(home_position, 0)),
    _target_id(target_id)
{
    _status_icon_sprite.set_z_order(-2);
    _status_icon_sprite.set_visible(false);
}

void Goblin::enter()
{
    _state = State::ROAM;
    _state_timer = roam_direction_frames;
    _roam_direction_index = 0;
    _attack_direction = Direction::DOWN;
    _detour_direction = Direction::DOWN;
    _attack_hit_registry.reset();
    _status_icon_frame = 0;
    _status_icon_timer = 0;
    _reset_local_avoidance();
    _status_icon = StatusIcon::NONE;
    _respawn_timer = 0;
    _respawning = false;
    _active = true;
    _set_telegraph_visible(false);
    apply_movement(_home_position, Direction::DOWN);
    set_visible(true);
}

void Goblin::set_home_position(const bn::fixed_point& position)
{
    _home_position = position;
}

void Goblin::set_respawn_enabled(bool enabled)
{
    _respawn_enabled = enabled;
    if(! _respawn_enabled)
    {
        _respawning = false;
        _respawn_timer = 0;
    }
}

void Goblin::update(const WorldBox& player_hurtbox, const WorldBox& player_pushbox,
                    const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(! _active)
    {
        if(_respawn_enabled)
        {
            _update_respawn(blocking_pushboxes);
        }
        return;
    }

    _update_timed_status_icon();

    switch(_state)
    {
    case State::ROAM:
        if(within_distance(position(), player_pushbox.center, discovery_distance))
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
        _update_chase(player_hurtbox, blocking_pushboxes);
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
        _update_return(player_pushbox, blocking_pushboxes);
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
    if(! _active)
    {
        return;
    }

    int damage = attack.try_hit(_target_id, position(), collision_body().hurtbox);
    if(damage > 0)
    {
        hit_effects.spawn(world_hurtbox().center);
        _die();
    }
}

void Goblin::resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                                HitEffectManager& hit_effects)
{
    if(! attack_active() || _attack_hit_registry.contains(player_target_id))
    {
        return;
    }

    WorldBox hitbox = attack_hitbox(position(), _attack_direction);
    WorldBox hurtbox = world_box(player_position, player_hurtbox.box);
    if(touches_or_intersects(hitbox, hurtbox) && _attack_hit_registry.add(player_target_id))
    {
        hit_effects.spawn(hurtbox.center);
    }
}

bool Goblin::active() const
{
    return _active;
}

bool Goblin::attack_active() const
{
    return _active && _state == State::ACTIVE;
}

Goblin::State Goblin::state() const
{
    return _state;
}

WorldBox Goblin::world_hurtbox() const
{
    return world_box(position(), collision_body().hurtbox.box);
}

WorldBox Goblin::world_pushbox() const
{
    return world_box(position(), collision_body().pushbox.box);
}

WorldBox Goblin::movement_obstacle_query_area() const
{
    if(_active)
    {
        return world_pushbox();
    }

    return expanded_box(world_box(_home_position, collision_body().pushbox.box), respawn_clearance);
}

void Goblin::append_collision_debug_boxes(const WorldBox& player_hurtbox, CollisionDebugBoxList& boxes) const
{
    if(! _active)
    {
        return;
    }

    boxes.add(world_hurtbox(), CollisionDebugBoxType::HURTBOX);
    boxes.add(world_pushbox(), CollisionDebugBoxType::PUSHBOX);
    if(_state == State::CHASE)
    {
        Direction attack_direction = nearest_direction(position(), player_hurtbox.center, direction());
        boxes.add(commit_box(attack_hitbox(position(), attack_direction)), CollisionDebugBoxType::COMMIT_BOX);
    }
    else if(_state == State::TELEGRAPH)
    {
        boxes.add(commit_box(attack_hitbox(position(), _attack_direction)), CollisionDebugBoxType::COMMIT_BOX);
    }
    else if(attack_active())
    {
        boxes.add(attack_hitbox(position(), _attack_direction), CollisionDebugBoxType::HITBOX);
    }
}

void Goblin::_update_roam(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    _move_direction(roam_directions[_roam_direction_index], roam_speed, blocking_pushboxes, true);
    --_state_timer;
    if(_state_timer == 0)
    {
        _roam_direction_index = (_roam_direction_index + 1) % roam_directions.size();
        _state_timer = roam_direction_frames;
    }
}

void Goblin::_update_chase(const WorldBox& player_hurtbox,
                           const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(! within_distance(position(), player_hurtbox.center, disengage_distance) && _detour_frames == 0)
    {
        _reset_local_avoidance();
        _state = State::RETURN;
        _status_icon_timer = return_question_frames;
        _set_awareness_icon(StatusIcon::RETURN_QUESTION);
        return;
    }

    Direction attack_direction = nearest_direction(position(), player_hurtbox.center, direction());
    if(touches_or_intersects(commit_box(attack_hitbox(position(), attack_direction)), player_hurtbox))
    {
        _start_attack(attack_direction);
        return;
    }

    _move_toward_with_local_avoidance(player_hurtbox.center, chase_speed, blocking_pushboxes);
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

void Goblin::_update_return(const WorldBox& player_pushbox,
                            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(within_distance(position(), player_pushbox.center, discovery_distance))
    {
        _state = State::CHASE;
        _status_icon_timer = discovery_flash_frames;
        _set_awareness_icon(StatusIcon::DISCOVERY_FLASH);
        return;
    }

    if(within_distance(position(), _home_position, 1))
    {
        _state = State::ROAM;
        _state_timer = roam_direction_frames;
        _status_icon_timer = 0;
        _set_telegraph_visible(false);
        return;
    }

    _move_toward(_home_position, chase_speed, blocking_pushboxes);
}

void Goblin::_start_attack(Direction direction)
{
    _reset_local_avoidance();
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
    if(! _active)
    {
        return;
    }

    _attack_hit_registry.reset();
    _status_icon_timer = 0;
    _set_telegraph_visible(false);
    _state = State::DEAD;
    _state_timer = 0;
    _respawn_timer = _respawn_enabled ? respawn_delay_ticks : 0;
    _reset_local_avoidance();
    _respawning = _respawn_enabled;
    _active = false;
    set_visible(false);
}

void Goblin::_update_respawn(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    if(! _respawning)
    {
        return;
    }

    if(_respawn_timer > 0)
    {
        --_respawn_timer;
        return;
    }

    if(_respawn_position_is_safe(blocking_pushboxes))
    {
        enter();
    }
}

bool Goblin::_respawn_position_is_safe(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes) const
{
    WorldBox spawn_pushbox = world_box(_home_position, collision_body().pushbox.box);
    WorldBox safe_spawn_pushbox = expanded_box(spawn_pushbox, respawn_clearance);

    for(int index = 0; index < blocking_pushboxes.count; ++index)
    {
        if(touches_or_intersects(safe_spawn_pushbox, blocking_pushboxes.boxes[index]))
        {
            return false;
        }
    }

    return true;
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

Goblin::MovementResult Goblin::_resolve_move_direction(
        Direction direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
        bool constrain_to_home, bn::fixed_point& resolved_position) const
{
    int horizontal;
    int vertical;
    direction_components(direction, horizontal, vertical);
    bn::fixed step = speed;
    if(horizontal && vertical)
    {
        step *= diagonal_ratio;
    }

    bn::fixed_point next(position().x() + horizontal * step, position().y() + vertical * step);
    if(constrain_to_home)
    {
        next.set_x(clamp(next.x(), _home_position.x() - home_radius, _home_position.x() + home_radius));
        next.set_y(clamp(next.y(), _home_position.y() - home_radius, _home_position.y() + home_radius));
    }

    resolved_position = resolve_movement(
            position(), next - position(), collision_body().pushbox,
            blocking_pushboxes, goblin_movement_bounds);
    if(resolved_position == position())
    {
        return MovementResult::BLOCKED;
    }

    return resolved_position == next ? MovementResult::FULL : MovementResult::PARTIAL;
}

Goblin::MovementResult Goblin::_try_move_direction(
        Direction direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
        bool constrain_to_home)
{
    bn::fixed_point resolved_position;
    MovementResult result = _resolve_move_direction(
            direction, speed, blocking_pushboxes, constrain_to_home, resolved_position);
    if(result == MovementResult::BLOCKED)
    {
        return result;
    }

    apply_movement(resolved_position, direction);
    return result;
}

void Goblin::_move_direction(Direction direction, bn::fixed speed,
                             const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
                             bool constrain_to_home)
{
    if(_try_move_direction(direction, speed, blocking_pushboxes, constrain_to_home) == MovementResult::BLOCKED)
    {
        apply_movement(position(), direction);
    }
}

void Goblin::_move_toward(const bn::fixed_point& target, bn::fixed speed,
                          const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    Direction movement_direction = direction_from_components(
            sign(target.x() - position().x()), sign(target.y() - position().y()), direction());
    _reset_local_avoidance();
    _move_direction(movement_direction, speed, blocking_pushboxes, false);
}

void Goblin::_reset_local_avoidance()
{
    _stuck_frames = 0;
    _detour_frames = 0;
}

void Goblin::_start_local_detour(
        const bn::fixed_point& target, Direction desired_direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    bn::array<Direction, 4> candidates = detour_candidates(desired_direction, _target_id % 2 == 0);
    bool found_candidate = false;
    bn::fixed best_distance = 0;

    for(Direction candidate : candidates)
    {
        bn::fixed_point resolved_position;
        if(_resolve_move_direction(candidate, speed, blocking_pushboxes, false, resolved_position) ==
           MovementResult::FULL)
        {
            bn::fixed candidate_distance = squared_distance(resolved_position, target);
            if(! found_candidate || candidate_distance < best_distance)
            {
                found_candidate = true;
                best_distance = candidate_distance;
                _detour_direction = candidate;
            }
        }
    }

    if(found_candidate)
    {
        _detour_frames = max_detour_frames;
    }
}

void Goblin::_move_toward_with_local_avoidance(
        const bn::fixed_point& target, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes)
{
    Direction desired_direction = direction_from_components(
            sign(target.x() - position().x()), sign(target.y() - position().y()), direction());

    if(_detour_frames > 0)
    {
        bn::fixed_point direct_position;
        if(_resolve_move_direction(desired_direction, speed, blocking_pushboxes, false, direct_position) ==
           MovementResult::FULL)
        {
            _reset_local_avoidance();
            apply_movement(direct_position, desired_direction);
            return;
        }

        MovementResult detour_result = _try_move_direction(_detour_direction, speed, blocking_pushboxes, false);
        _detour_frames = next_detour_frames(_detour_frames);
        if(detour_result == MovementResult::BLOCKED)
        {
            _detour_frames = 0;
        }

        if(_detour_frames == 0)
        {
            _stuck_frames = 0;
        }
        return;
    }

    MovementResult direct_result = _try_move_direction(desired_direction, speed, blocking_pushboxes, false);
    if(direct_result == MovementResult::FULL)
    {
        _stuck_frames = 0;
        return;
    }

    _stuck_frames = next_stuck_frames(_stuck_frames, false);
    if(_stuck_frames == stuck_frames_before_detour)
    {
        _start_local_detour(target, desired_direction, speed, blocking_pushboxes);
    }
}
