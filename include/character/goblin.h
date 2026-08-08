#ifndef CHARACTER_GOBLIN_H
#define CHARACTER_GOBLIN_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

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

    explicit Goblin(const bn::fixed_point& home_position);

    void enter();
    void update(const WorldBox& player_pushbox);
    void resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects);
    void resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                            HitEffectManager& hit_effects);

    [[nodiscard]] bool active() const;
    [[nodiscard]] bool attack_active() const;
    [[nodiscard]] State state() const;
    [[nodiscard]] WorldBox world_hurtbox() const;
    [[nodiscard]] WorldBoxList<3> active_pushboxes() const;
    void append_collision_debug_boxes(CollisionDebugBoxList& boxes) const;

private:
    void _update_roam();
    void _update_chase(const WorldBox& player_pushbox);
    void _update_telegraph();
    void _update_active();
    void _update_recovery();
    void _update_return();
    void _start_attack(const bn::fixed_point& player_position);
    void _finish_attack();
    void _die();
    void _set_telegraph_visible(bool visible);
    void _move_direction(Direction direction, bn::fixed speed, const WorldBox* blocking_pushbox,
                         bool constrain_to_home);
    void _move_toward(const bn::fixed_point& target, bn::fixed speed, const WorldBox* blocking_pushbox);

    bn::fixed_point _home_position;
    bn::optional<bn::sprite_ptr> _telegraph_sprite;
    AttackHitRegistry _attack_hit_registry;
    Direction _attack_direction = Direction::DOWN;
    State _state = State::ROAM;
    int _state_timer = 0;
    int _roam_direction_index = 0;
    bool _active = true;
};

#endif
