#include "character/swordsman.h"

#include "bn_sprite_items_swordsman_8dir_sheet.h"

namespace
{
    constexpr bn::fixed swordsman_movement_speed(1);
    constexpr Pushbox swordsman_body_pushbox = { { 0, 1, 8, 8 } };
    constexpr CollisionBody swordsman_collision_body = {
        { { 0, 0, 10, 10 } },
        swordsman_body_pushbox
    };

    static_assert(swordsman_body_pushbox.box.offset_y == 1);
    static_assert(swordsman_body_pushbox.box.width == 8 && swordsman_body_pushbox.box.height == 8);
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
