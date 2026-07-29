#ifndef COMBAT_MELEE_SWORDSMAN_ATTACK_H
#define COMBAT_MELEE_SWORDSMAN_ATTACK_H

#include "combat/attack_behavior.h"
#include "combat/melee/melee_hitbox.h"

class SwordsmanAttack : public AttackBehavior
{
public:
    static constexpr int cooldown_frames = 33;
    static constexpr int hitbox_active_frames = 6;
    static constexpr int attack_power = 1;

    [[nodiscard]] bool can_attack() const final;
    bool try_attack(const AttackContext& context) final;
    void update() final;

    [[nodiscard]] int try_hit(int target_id, const bn::fixed_point& target_position,
                              const Hurtbox& target_hurtbox);

private:
    MeleeHitbox _hitbox;
    int _cooldown_remaining = 0;
};

#endif
