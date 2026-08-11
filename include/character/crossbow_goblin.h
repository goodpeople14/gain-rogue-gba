#ifndef CHARACTER_CROSSBOW_GOBLIN_H
#define CHARACTER_CROSSBOW_GOBLIN_H

#include "bn_array.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"

#include "character/character.h"
#include "combat/collision/collision_debug_box.h"

class CrossbowProjectilePool;
class HitEffectManager;
class SwordsmanAttack;

class CrossbowGoblin final : public Character
{
public:
    enum class State { ROAM, CHASE, TELEGRAPH, RECOVERY, RETURN, DEAD };

    CrossbowGoblin(const bn::fixed_point& home_position, int target_id);

    void enter();
    void set_respawn_enabled(bool enabled);
    void update(const WorldBox& player_hurtbox, const WorldBox& player_pushbox,
                const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
                CrossbowProjectilePool& projectiles);
    void resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects);

    [[nodiscard]] bool active() const;
    [[nodiscard]] WorldBox world_hurtbox() const;
    [[nodiscard]] WorldBox world_pushbox() const;
    [[nodiscard]] WorldBox movement_obstacle_query_area() const;
    void append_collision_debug_boxes(const WorldBox& player_hurtbox, CollisionDebugBoxList& boxes) const;

private:
    enum class StatusIcon { NONE, DISCOVERY_FLASH, TELEGRAPH, RECOVERY_HOURGLASS, RETURN_QUESTION };

    void _update_roam(const WorldBoxList<max_movement_obstacles>& blockers);
    void _update_chase(const WorldBox& player_hurtbox, const WorldBoxList<max_movement_obstacles>& blockers);
    void _update_telegraph(const WorldBox& player_hurtbox, CrossbowProjectilePool& projectiles);
    void _update_return(const WorldBox& player_pushbox, const WorldBoxList<max_movement_obstacles>& blockers);
    void _start_attack(Direction direction);
    void _die();
    void _update_respawn(const WorldBoxList<max_movement_obstacles>& blockers);
    [[nodiscard]] bool _respawn_position_is_safe(const WorldBoxList<max_movement_obstacles>& blockers) const;
    void _move_direction(Direction direction, bn::fixed speed, const WorldBoxList<max_movement_obstacles>& blockers,
                         bool constrain_to_home);
    void _move_toward(const bn::fixed_point& target, const WorldBoxList<max_movement_obstacles>& blockers);
    void _set_telegraph_visible(bool visible);
    void _set_recovery_hourglass_visible(bool visible);
    void _set_awareness_icon(StatusIcon icon);
    void _show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                           const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame);
    void _update_timed_status_icon();

    bn::fixed_point _home_position;
    bn::fixed_point _locked_target;
    bn::array<bn::sprite_tiles_ptr, 3> _awareness_icon_tiles;
    bn::sprite_palette_ptr _awareness_icon_palette;
    bn::sprite_tiles_ptr _telegraph_tiles;
    bn::sprite_palette_ptr _telegraph_palette;
    bn::array<bn::sprite_tiles_ptr, 2> _recovery_hourglass_tiles;
    bn::sprite_palette_ptr _recovery_hourglass_palette;
    bn::sprite_ptr _status_icon_sprite;
    int _target_id;
    Direction _attack_direction = Direction::DOWN;
    Direction _alignment_direction = Direction::DOWN;
    State _state = State::ROAM;
    StatusIcon _status_icon = StatusIcon::NONE;
    int _state_timer = 0;
    int _roam_direction_index = 0;
    int _status_icon_frame = 0;
    int _status_icon_timer = 0;
    int _respawn_timer = 0;
    bool _respawning = false;
    bool _respawn_enabled = true;
    bool _active = true;
};

#endif
