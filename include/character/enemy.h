#ifndef CHARACTER_ENEMY_H
#define CHARACTER_ENEMY_H

#include "character/character.h"
#include "world/battlefield.h"

class Enemy : public Character
{
public:
    void set_home_position(const bn::fixed_point& position);
    void set_respawn_enabled(bool enabled);

    [[nodiscard]] bool active() const;
    [[nodiscard]] WorldBox world_hurtbox() const;
    [[nodiscard]] WorldBox world_pushbox() const;
    [[nodiscard]] WorldBox movement_obstacle_query_area() const;

    [[nodiscard]] static constexpr bool within_distance(
            const bn::fixed_point& first, const bn::fixed_point& second, int distance)
    {
        bn::fixed x = first.x() - second.x();
        bn::fixed y = first.y() - second.y();
        return (x * x) + (y * y) <= distance * distance;
    }

protected:
    Enemy(const bn::sprite_item& sprite_item, const bn::fixed_point& initial_position,
          Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body,
          const CharacterDefinition& definition, const MovementBounds& movement_bounds, int target_id);

    void enter_enemy();
    void deactivate_enemy();
    void hide_enemy();
    void defeat_enemy(int respawn_delay_ticks);

    [[nodiscard]] int target_id() const;
    [[nodiscard]] const bn::fixed_point& home_position() const;
    [[nodiscard]] bool respawn_ready(const WorldBoxList<max_movement_obstacles>& obstacles);
    [[nodiscard]] bool local_detour_active() const;

    [[nodiscard]] static Direction direction_toward(
            const bn::fixed_point& origin, const bn::fixed_point& target, Direction fallback);

    void reset_local_avoidance();
    void move_direction(Direction direction, bn::fixed speed,
                        const WorldBoxList<max_movement_obstacles>& obstacles,
                        bool constrain_to_home, int home_radius);
    void move_toward(const bn::fixed_point& target, bn::fixed speed,
                     const WorldBoxList<max_movement_obstacles>& obstacles);
    void move_toward_with_local_avoidance(
            const bn::fixed_point& target, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& obstacles);

private:
    enum class MovementResult
    {
        BLOCKED,
        PARTIAL,
        FULL
    };

    [[nodiscard]] MovementResult _resolve_move_direction(
            Direction direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& obstacles,
            bool constrain_to_home, int home_radius, bn::fixed_point& resolved_position) const;
    [[nodiscard]] MovementResult _try_move_direction(
            Direction direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& obstacles,
            bool constrain_to_home, int home_radius);
    void _start_local_detour(
            const bn::fixed_point& target, Direction desired_direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& obstacles);
    [[nodiscard]] bool _respawn_position_is_safe(
            const WorldBoxList<max_movement_obstacles>& obstacles) const;

    MovementBounds _movement_bounds;
    bn::fixed_point _home_position;
    int _target_id;
    Direction _detour_direction = Direction::DOWN;
    int _stuck_frames = 0;
    int _detour_frames = 0;
    int _respawn_timer = 0;
    bool _respawning = false;
    bool _respawn_enabled = true;
    bool _active = true;
};

#endif
