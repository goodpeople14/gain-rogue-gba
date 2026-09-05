#include "character/enemy.h"

#include "bn_array.h"

#include "combat/collision/collision_math.h"
#include "combat/collision/movement_collision.h"

#if defined(GAIN_PERF_DEBUG_LOGS)
    #include "debug/perf_stats.h"
#endif

namespace
{
    constexpr int respawn_clearance = 2;
    constexpr int stuck_frames_before_detour = 6;
    constexpr int max_detour_frames = 32;
    constexpr bn::fixed diagonal_ratio(0.70710678f);

    [[nodiscard]] constexpr int sign(bn::fixed value)
    {
        return value > 0 ? 1 : value < 0 ? -1 : 0;
    }

    [[nodiscard]] constexpr Direction direction_from_components(
            int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical)
        {
            return fallback;
        }

        if(vertical > 0)
        {
            return horizontal < 0 ? Direction::DOWN_LEFT :
                   horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN;
        }

        if(vertical < 0)
        {
            return horizontal < 0 ? Direction::UP_LEFT :
                   horizontal > 0 ? Direction::UP_RIGHT : Direction::UP;
        }

        return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
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
        return Direction(index < 0 ? index + 8 : index);
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

    [[nodiscard]] constexpr bn::fixed clamp(bn::fixed value, bn::fixed minimum, bn::fixed maximum)
    {
        return value < minimum ? minimum : value > maximum ? maximum : value;
    }

    [[nodiscard]] constexpr WorldBox expanded_box(const WorldBox& box, int clearance)
    {
        return { box.center, box.width + clearance * 2, box.height + clearance * 2 };
    }

    [[nodiscard]] bn::fixed squared_distance(const bn::fixed_point& first, const bn::fixed_point& second)
    {
        bn::fixed x = first.x() - second.x();
        bn::fixed y = first.y() - second.y();
        return (x * x) + (y * y);
    }

    static_assert(direction_from_components(-1, -1, Direction::DOWN) == Direction::UP_LEFT);
    static_assert(direction_from_components(0, -1, Direction::DOWN) == Direction::UP);
    static_assert(direction_from_components(1, -1, Direction::DOWN) == Direction::UP_RIGHT);
    static_assert(direction_from_components(-1, 0, Direction::DOWN) == Direction::LEFT);
    static_assert(direction_from_components(1, 0, Direction::DOWN) == Direction::RIGHT);
    static_assert(direction_from_components(-1, 1, Direction::UP) == Direction::DOWN_LEFT);
    static_assert(direction_from_components(0, 1, Direction::UP) == Direction::DOWN);
    static_assert(direction_from_components(1, 1, Direction::UP) == Direction::DOWN_RIGHT);
    static_assert(direction_from_components(0, 0, Direction::LEFT) == Direction::LEFT);
    static_assert(detour_candidates(Direction::RIGHT, false)[0] == Direction::UP);
    static_assert(detour_candidates(Direction::RIGHT, false)[1] == Direction::DOWN);
    static_assert(detour_candidates(Direction::UP_RIGHT, false)[0] == Direction::UP);
    static_assert(detour_candidates(Direction::RIGHT, true)[0] == Direction::DOWN);
    static_assert(next_stuck_frames(0, false) == 1);
    static_assert(next_stuck_frames(stuck_frames_before_detour - 1, false) == stuck_frames_before_detour);
    static_assert(next_stuck_frames(stuck_frames_before_detour, true) == 0);
    static_assert(next_detour_frames(max_detour_frames) == max_detour_frames - 1);
    static_assert(next_detour_frames(1) == 0);
}

Enemy::Enemy(const bn::sprite_item& sprite_item, const bn::fixed_point& initial_position,
             Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body,
             const CharacterDefinition& definition, const MovementBounds& movement_bounds, int target_id) :
    Character(sprite_item, initial_position, direction, movement_speed, collision_body, definition),
    _movement_bounds(movement_bounds),
    _home_position(initial_position),
    _target_id(target_id)
{
}

void Enemy::set_home_position(const bn::fixed_point& position)
{
    _home_position = position;
}

void Enemy::set_respawn_enabled(bool enabled)
{
    _respawn_enabled = enabled;
    if(! enabled)
    {
        _respawning = false;
        _respawn_timer = 0;
    }
}

bool Enemy::active() const
{
    return _active;
}

bool Enemy::respawn_check_ready()
{
    if(! _respawning)
    {
        return false;
    }

    if(_respawn_timer > 0)
    {
        --_respawn_timer;
        return false;
    }

    return true;
}

int Enemy::actor_id() const
{
    return _target_id;
}

WorldBox Enemy::world_hurtbox() const
{
    return world_box(position(), collision_body().hurtbox.box);
}

WorldBox Enemy::world_pushbox() const
{
    return world_box(position(), collision_body().pushbox.box);
}

WorldBox Enemy::movement_obstacle_query_area() const
{
    if(_active)
    {
        return world_pushbox();
    }

    return expanded_box(world_box(_home_position, collision_body().pushbox.box), respawn_clearance);
}

void Enemy::enter_enemy()
{
    reset_health();
    reset_local_avoidance();
    _respawn_timer = 0;
    _respawning = false;
    _active = true;
    apply_movement(_home_position, Direction::DOWN);
    set_visible(true);
}

void Enemy::deactivate_enemy()
{
    reset_local_avoidance();
    _active = false;
    _respawning = false;
    _respawn_timer = 0;
    set_visible(false);
}

void Enemy::hide_enemy()
{
    set_visible(false);
}

void Enemy::defeat_enemy(int respawn_delay_ticks)
{
    reset_local_avoidance();
    _respawn_timer = _respawn_enabled ? respawn_delay_ticks : 0;
    _respawning = _respawn_enabled;
    _active = false;
    set_visible(false);
}

int Enemy::target_id() const
{
    return _target_id;
}

const bn::fixed_point& Enemy::home_position() const
{
    return _home_position;
}

bool Enemy::respawn_position_ready(const WorldBoxList<max_movement_obstacles>& obstacles) const
{
    return _respawning && _respawn_position_is_safe(obstacles);
}

bool Enemy::local_detour_active() const
{
    return _detour_frames > 0;
}

Direction Enemy::direction_toward(
        const bn::fixed_point& origin, const bn::fixed_point& target, Direction fallback)
{
    return direction_from_components(sign(target.x() - origin.x()), sign(target.y() - origin.y()), fallback);
}

MovementIntent Enemy::movement_intent(Direction movement_direction, bn::fixed speed) const
{
    int horizontal;
    int vertical;
    direction_components(movement_direction, horizontal, vertical);
    bn::fixed step = horizontal && vertical ? speed * diagonal_ratio : speed;
    return { { horizontal * step, vertical * step }, movement_direction, true };
}

void Enemy::reset_local_avoidance()
{
    _stuck_frames = 0;
    _detour_frames = 0;
}

void Enemy::move_direction(Direction movement_direction, bn::fixed speed,
                           const WorldBoxList<max_movement_obstacles>& obstacles,
                           bool constrain_to_home, int home_radius)
{
    if(_try_move_direction(movement_direction, speed, obstacles, constrain_to_home, home_radius) ==
       MovementResult::BLOCKED)
    {
        apply_movement(position(), movement_direction);
    }
}

void Enemy::move_toward(const bn::fixed_point& target, bn::fixed speed,
                        const WorldBoxList<max_movement_obstacles>& obstacles)
{
    Direction desired_direction = direction_toward(position(), target, direction());
    reset_local_avoidance();
    move_direction(desired_direction, speed, obstacles, false, 0);
}

void Enemy::move_toward_with_local_avoidance(
        const bn::fixed_point& target, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& obstacles)
{
    Direction desired_direction = direction_toward(position(), target, direction());

    if(_detour_frames > 0)
    {
        bn::fixed_point direct_position;
        if(_resolve_move_direction(desired_direction, speed, obstacles, false, 0, direct_position) ==
           MovementResult::FULL)
        {
            reset_local_avoidance();
            apply_movement(direct_position, desired_direction);
            return;
        }

        MovementResult detour_result = _try_move_direction(
                _detour_direction, speed, obstacles, false, 0);
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

    MovementResult direct_result = _try_move_direction(desired_direction, speed, obstacles, false, 0);
    if(direct_result == MovementResult::FULL)
    {
        _stuck_frames = 0;
        return;
    }

    _stuck_frames = next_stuck_frames(_stuck_frames, false);
    if(_stuck_frames == stuck_frames_before_detour)
    {
        _start_local_detour(target, desired_direction, speed, obstacles);
    }
}

Enemy::MovementResult Enemy::_resolve_move_direction(
        Direction movement_direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& obstacles,
        bool constrain_to_home, int home_radius, bn::fixed_point& resolved_position) const
{
#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().resolve_movement_calls;
#endif
    int horizontal;
    int vertical;
    direction_components(movement_direction, horizontal, vertical);
    bn::fixed step = horizontal && vertical ? speed * diagonal_ratio : speed;
    bn::fixed_point next(position().x() + horizontal * step, position().y() + vertical * step);
    if(constrain_to_home)
    {
        next.set_x(clamp(next.x(), _home_position.x() - home_radius, _home_position.x() + home_radius));
        next.set_y(clamp(next.y(), _home_position.y() - home_radius, _home_position.y() + home_radius));
    }

    resolved_position = resolve_movement(
            position(), next - position(), collision_body().pushbox, obstacles, _movement_bounds);
    if(resolved_position == position())
    {
#if defined(GAIN_PERF_DEBUG_LOGS)
        ++perf_stats().movement_blocked;
#endif
        return MovementResult::BLOCKED;
    }

    if(resolved_position == next)
    {
#if defined(GAIN_PERF_DEBUG_LOGS)
        ++perf_stats().movement_full;
#endif
        return MovementResult::FULL;
    }

#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().movement_partial;
#endif
    return MovementResult::PARTIAL;
}

Enemy::MovementResult Enemy::_try_move_direction(
        Direction movement_direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& obstacles,
        bool constrain_to_home, int home_radius)
{
    bn::fixed_point resolved_position;
    MovementResult result = _resolve_move_direction(
            movement_direction, speed, obstacles, constrain_to_home, home_radius, resolved_position);
    if(result != MovementResult::BLOCKED)
    {
        apply_movement(resolved_position, movement_direction);
    }
    return result;
}

void Enemy::_start_local_detour(
        const bn::fixed_point& target, Direction desired_direction, bn::fixed speed,
        const WorldBoxList<max_movement_obstacles>& obstacles)
{
#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().detour_start_count;
#endif
    bn::array<Direction, 4> candidates = detour_candidates(desired_direction, _target_id % 2 == 0);
    bool found_candidate = false;
    bn::fixed best_distance = 0;

    for(Direction candidate : candidates)
    {
#if defined(GAIN_PERF_DEBUG_LOGS)
        ++perf_stats().detour_candidate_checks;
#endif
        bn::fixed_point resolved_position;
        if(_resolve_move_direction(candidate, speed, obstacles, false, 0, resolved_position) == MovementResult::FULL)
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

bool Enemy::_respawn_position_is_safe(const WorldBoxList<max_movement_obstacles>& obstacles) const
{
    WorldBox safe = expanded_box(world_box(_home_position, collision_body().pushbox.box), respawn_clearance);
    for(int index = 0; index < obstacles.count; ++index)
    {
        if(touches_or_intersects(safe, obstacles.boxes[index]))
        {
            return false;
        }
    }
    return true;
}
