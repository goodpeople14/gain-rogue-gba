#ifndef COMBAT_MELEE_SWORDSMAN_ATTACK_H
#define COMBAT_MELEE_SWORDSMAN_ATTACK_H

#include "combat/melee/melee_hitbox.h"
#include "combat/melee/swordsman_slash_effect.h"

class SwordsmanAttack
{
public:
    static constexpr int cooldown_frames = 33;
    static constexpr int hitbox_active_frames = 6;
    static constexpr int attack_power = 1;

    [[nodiscard]] bool can_attack() const;
    bool try_attack(const AttackContext& context);
    void update(const bn::fixed_point& owner_position);

    [[nodiscard]] int try_hit(int target_id, const bn::fixed_point& target_position,
                              const Hurtbox& target_hurtbox);

private:
    MeleeHitbox _hitbox;
    SwordsmanSlashEffect _slash_effect;
    int _cooldown_remaining = 0;
};

#endif
