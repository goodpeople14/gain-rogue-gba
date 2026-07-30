#ifndef COMBAT_ATTACK_BEHAVIOR_H
#define COMBAT_ATTACK_BEHAVIOR_H

#include "combat/attack_context.h"

class AttackBehavior
{
public:
    virtual ~AttackBehavior() = default;

    [[nodiscard]] virtual bool can_attack() const = 0;
    [[nodiscard]] virtual bool hides_character() const = 0;
    virtual bool try_attack(const AttackContext& context) = 0;
    virtual void update() = 0;
};

#endif
