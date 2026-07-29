#ifndef CHARACTER_H
#define CHARACTER_H

#include "bn_fixed.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"

#include "battlefield.h"
#include "combat/attack_behavior.h"
#include "direction.h"

class Character
{
public:
    Character(const bn::sprite_item& sprite_item, bn::fixed x, bn::fixed y,
              Direction direction, bn::fixed movement_speed, AttackBehavior& attack_behavior);

    void set_visible(bool visible);
    void move(bn::fixed delta_x, bn::fixed delta_y, Direction direction, const MovementBounds& bounds);
    bool attack();
    void update_attack();

    [[nodiscard]] bn::fixed movement_speed() const;
    [[nodiscard]] Direction direction() const;
    [[nodiscard]] bn::fixed_point position() const;

private:
    bn::sprite_ptr _sprite;
    bn::sprite_tiles_item _tiles_item;
    Direction _direction;
    bn::fixed _movement_speed;
    AttackBehavior& _attack_behavior;
};

#endif
