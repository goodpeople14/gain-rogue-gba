#include "character/swordsman.h"

#include "bn_sprite_items_swordsman_8dir_sheet.h"

namespace
{
    constexpr bn::fixed swordsman_movement_speed(1);
    constexpr CollisionBody swordsman_collision_body = {
        { { 0, 0, 10, 10 } },
        { { 0, 3, 8, 8 } }
    };
}

Swordsman::Swordsman(const bn::fixed_point& initial_position) :
    Character(bn::sprite_items::swordsman_8dir_sheet, initial_position, Direction::DOWN,
              swordsman_movement_speed, swordsman_collision_body)
{
}

bool Swordsman::try_attack()
{
    return _attack.try_attack({ position(), direction() });
}

void Swordsman::update()
{
    _attack.update(position());
}

SwordsmanAttack& Swordsman::melee_attack()
{
    return _attack;
}

const SwordsmanAttack& Swordsman::melee_attack() const
{
    return _attack;
}
