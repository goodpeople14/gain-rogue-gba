#ifndef CHARACTER_GOBLIN_H
#define CHARACTER_GOBLIN_H

#include "bn_array.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"

#include "character/character.h"
#include "combat/attack_hit_registry.h"
#include "combat/collision/collision_debug_box.h"

class HitEffectManager;
class SwordsmanAttack;

class Goblin final : public Character
{
public:
    enum class State
    {
        ROAM,
        CHASE,
        TELEGRAPH,
        ACTIVE,
        RECOVERY,
        RETURN,
        DEAD
    };

    static constexpr int max_hp = 1;

    Goblin(const bn::fixed_point& home_position, int target_id);

    void enter();
    void deactivate();
    void set_home_position(const bn::fixed_point& position);
    void set_respawn_enabled(bool enabled);
    void update(const WorldBox& player_hurtbox, const WorldBox& player_pushbox, bool player_on_same_layer,
                const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects);
    void resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                            HitEffectManager& hit_effects);

    [[nodiscard]] bool active() const;
    [[nodiscard]] bool attack_active() const;
    [[nodiscard]] State state() const;
    [[nodiscard]] WorldBox world_hurtbox() const;
    [[nodiscard]] WorldBox world_pushbox() const;
    [[nodiscard]] WorldBox movement_obstacle_query_area() const;
    void append_collision_debug_boxes(const WorldBox& player_hurtbox, CollisionDebugBoxList& boxes) const;

private:
    enum class StatusIcon
    {
        NONE,
        DISCOVERY_FLASH,
        TELEGRAPH,
        RECOVERY_HOURGLASS,
        RETURN_QUESTION
    };

    enum class MovementResult
    {
        BLOCKED,
        PARTIAL,
        FULL
    };

    void _update_roam(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _update_chase(const WorldBox& player_hurtbox,
                       const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _update_telegraph();
    void _update_active();
    void _update_recovery();
    void _update_return(const WorldBox& player_pushbox,
                        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _start_attack(Direction direction);
    void _finish_attack();
    void _die();
    void _update_respawn(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    [[nodiscard]] bool _respawn_position_is_safe(
            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes) const;
    void _set_telegraph_visible(bool visible);
    void _set_recovery_hourglass_visible(bool visible);
    void _set_awareness_icon(StatusIcon icon);
    void _show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                           const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame,
                           const bn::fixed_point& position);
    void _update_timed_status_icon();
    void _reset_local_avoidance();
    void _start_local_detour(
            const bn::fixed_point& target, Direction desired_direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _move_toward_with_local_avoidance(
            const bn::fixed_point& target, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    [[nodiscard]] MovementResult _resolve_move_direction(
            Direction direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
            bool constrain_to_home, bn::fixed_point& resolved_position) const;
    [[nodiscard]] MovementResult _try_move_direction(
            Direction direction, bn::fixed speed,
            const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
            bool constrain_to_home);
    void _move_direction(Direction direction, bn::fixed speed,
                         const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
                         bool constrain_to_home);
    void _move_toward(const bn::fixed_point& target, bn::fixed speed,
                      const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);

    bn::fixed_point _home_position;
    bn::array<bn::sprite_tiles_ptr, 3> _awareness_icon_tiles;
    bn::sprite_palette_ptr _awareness_icon_palette;
    bn::sprite_tiles_ptr _telegraph_tiles;
    bn::sprite_palette_ptr _telegraph_palette;
    bn::array<bn::sprite_tiles_ptr, 2> _recovery_hourglass_tiles;
    bn::sprite_palette_ptr _recovery_hourglass_palette;
    bn::sprite_ptr _status_icon_sprite;
    AttackHitRegistry _attack_hit_registry;
    int _target_id;
    Direction _attack_direction = Direction::DOWN;
    Direction _detour_direction = Direction::DOWN;
    State _state = State::ROAM;
    int _state_timer = 0;
    int _roam_direction_index = 0;
    int _status_icon_frame = 0;
    int _status_icon_timer = 0;
    int _stuck_frames = 0;
    int _detour_frames = 0;
    StatusIcon _status_icon = StatusIcon::NONE;
    int _respawn_timer = 0;
    bool _respawning = false;
    bool _respawn_enabled = true;
    bool _active = true;
};

#endif
