#include "character/swordsman.h"

#include "bn_sprite_items_swordsman_8dir_sheet.h"

namespace
{
    constexpr CharacterDefinition samurai_definition = {
        CharacterId::SAMURAI, "SAMURAI", 7, 3
    };
    constexpr bn::fixed swordsman_movement_speed(1);
    constexpr Pushbox swordsman_body_pushbox = { { 0, 1, 8, 8 } };
    constexpr CollisionBody swordsman_collision_body = {
        { { 0, 0, 10, 10 } },
        swordsman_body_pushbox
    };

    static_assert(swordsman_body_pushbox.box.offset_y == 1);
    static_assert(swordsman_body_pushbox.box.width == 8 && swordsman_body_pushbox.box.height == 8);
    static_assert(world_foot_position({ 0, 0 }, swordsman_body_pushbox) == bn::fixed_point(0, 5));
    static_assert(samurai_definition.id == CharacterId::SAMURAI);
    static_assert(samurai_definition.display_name_length == 7);
    static_assert(samurai_definition.max_hp == 3);
}

Swordsman::Swordsman(const bn::fixed_point& initial_position) :
    Character(bn::sprite_items::swordsman_8dir_sheet, initial_position, Direction::DOWN,
              swordsman_movement_speed, swordsman_collision_body, samurai_definition)
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
