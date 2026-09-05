#ifndef CHARACTER_GOBLIN_H
#define CHARACTER_GOBLIN_H

#include "bn_array.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"

#include "character/enemy.h"
#include "combat/attack_hit_registry.h"
#include "combat/collision/collision_debug_box.h"

class HitEffectManager;
class SwordsmanAttack;

class Goblin final : public Enemy
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

    Goblin(const bn::fixed_point& home_position, int target_id);

    void enter();
    void deactivate();
    void hide();
    [[nodiscard]] MovementIntent plan_movement(
            const bn::fixed_point& player_foot_position, bool player_on_same_layer) const;
    void update(const bn::fixed_point& player_foot_position, bool player_on_same_layer,
                const MovementIntent& movement,
                const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects);
    [[nodiscard]] int resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                                         HitEffectManager& hit_effects);

    [[nodiscard]] bool attack_active() const;
    [[nodiscard]] State state() const;
    void append_debug_shapes(
            CollisionDebugBoxList& boxes, CollisionDebugRadiusList& radii) const final;

private:
    enum class StatusIcon
    {
        NONE,
        DISCOVERY_FLASH,
        TELEGRAPH,
        RECOVERY_HOURGLASS,
        RETURN_QUESTION
    };

    void _update_roam(const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _update_chase(const bn::fixed_point& player_foot_position, bool movement_planned,
                       const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _update_telegraph();
    void _update_active();
    void _update_recovery();
    void _update_return(const bn::fixed_point& player_foot_position, bool movement_planned,
                        const WorldBoxList<max_movement_obstacles>& blocking_pushboxes);
    void _start_attack(Direction direction);
    void _finish_attack();
    void _die();
    void _set_telegraph_visible(bool visible);
    void _set_recovery_hourglass_visible(bool visible);
    void _set_awareness_icon(StatusIcon icon);
    void _show_status_icon(const bn::sprite_item& item, const bn::sprite_tiles_ptr& tiles,
                           const bn::sprite_palette_ptr& palette, StatusIcon icon, int frame,
                           const bn::fixed_point& position);
    void _update_timed_status_icon();
    bn::array<bn::sprite_tiles_ptr, 3> _awareness_icon_tiles;
    bn::sprite_palette_ptr _awareness_icon_palette;
    bn::sprite_tiles_ptr _telegraph_tiles;
    bn::sprite_palette_ptr _telegraph_palette;
    bn::array<bn::sprite_tiles_ptr, 2> _recovery_hourglass_tiles;
    bn::sprite_palette_ptr _recovery_hourglass_palette;
    bn::sprite_ptr _status_icon_sprite;
    AttackHitRegistry _attack_hit_registry;
    Direction _attack_direction = Direction::DOWN;
    State _state = State::ROAM;
    int _state_timer = 0;
    int _roam_direction_index = 0;
    int _status_icon_frame = 0;
    int _status_icon_timer = 0;
    StatusIcon _status_icon = StatusIcon::NONE;
};

#endif
