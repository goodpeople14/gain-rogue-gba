#ifndef CHARACTER_CROSSBOW_GOBLIN_H
#define CHARACTER_CROSSBOW_GOBLIN_H

#include "bn_array.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"

#include "character/enemy.h"
#include "combat/collision/collision_debug_box.h"

class CrossbowProjectilePool;
class HitEffectManager;
class SwordsmanAttack;

class CrossbowGoblin final : public Enemy
{
public:
    enum class State { ROAM, CHASE, TELEGRAPH, RECOVERY, RETURN, DEAD };
    CrossbowGoblin(const bn::fixed_point& home_position, int target_id);

    void enter();
    void deactivate();
    void hide();
    [[nodiscard]] MovementIntent plan_movement(const bn::fixed_point& player_foot_position) const;
    void update(const WorldBox& player_hurtbox, const bn::fixed_point& player_foot_position,
                const MovementIntent& movement,
                const WorldBoxList<max_movement_obstacles>& blocking_pushboxes,
                CrossbowProjectilePool& projectiles);
    void resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects);

    [[nodiscard]] State state() const;
    void append_debug_shapes(
            CollisionDebugBoxList& boxes, CollisionDebugRadiusList& radii) const final;

private:
    enum class StatusIcon { NONE, DISCOVERY_FLASH, TELEGRAPH, RECOVERY_HOURGLASS, RETURN_QUESTION };

    void _update_roam(const WorldBoxList<max_movement_obstacles>& blockers);
    void _update_chase(const WorldBox& player_hurtbox, const bn::fixed_point& player_foot_position,
                       bool movement_planned,
                       const WorldBoxList<max_movement_obstacles>& blockers);
    void _update_telegraph(const WorldBox& player_hurtbox, CrossbowProjectilePool& projectiles);
    void _update_return(const bn::fixed_point& player_foot_position, bool movement_planned,
                        const WorldBoxList<max_movement_obstacles>& blockers);
    void _start_attack(Direction direction);
    void _die();
    void _set_telegraph_visible(bool visible);
    void _set_recovery_hourglass_visible(bool visible);
    void _set_awareness_icon(StatusIcon icon);
    void _show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                           const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame);
    void _update_timed_status_icon();

    bn::fixed_point _locked_target;
    bn::array<bn::sprite_tiles_ptr, 3> _awareness_icon_tiles;
    bn::sprite_palette_ptr _awareness_icon_palette;
    bn::sprite_tiles_ptr _telegraph_tiles;
    bn::sprite_palette_ptr _telegraph_palette;
    bn::array<bn::sprite_tiles_ptr, 2> _recovery_hourglass_tiles;
    bn::sprite_palette_ptr _recovery_hourglass_palette;
    bn::sprite_ptr _status_icon_sprite;
    Direction _attack_direction = Direction::DOWN;
    State _state = State::ROAM;
    StatusIcon _status_icon = StatusIcon::NONE;
    int _state_timer = 0;
    int _roam_direction_index = 0;
    int _status_icon_frame = 0;
    int _status_icon_timer = 0;
};

#endif
