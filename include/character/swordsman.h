#ifndef CHARACTER_SWORDSMAN_H
#define CHARACTER_SWORDSMAN_H

#include "character/character.h"
#include "combat/melee/swordsman_attack.h"

class Swordsman final : public Character
{
public:
    explicit Swordsman(const bn::fixed_point& initial_position);

    bool try_attack();
    void update();

    [[nodiscard]] SwordsmanAttack& melee_attack();
    [[nodiscard]] const SwordsmanAttack& melee_attack() const;

private:
    SwordsmanAttack _attack;
};

#endif
