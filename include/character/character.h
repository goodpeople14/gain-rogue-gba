#ifndef CHARACTER_H
#define CHARACTER_H

#include "bn_fixed.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"

#include "combat/attack_behavior.h"
#include "combat/collision/collision_body.h"
#include "game/direction.h"
#include "world/battlefield.h"

class Character
{
public:
    Character(const bn::sprite_item& sprite_item, bn::fixed x, bn::fixed y,
              Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body,
              AttackBehavior& attack_behavior);

    void set_visible(bool visible);
    void apply_movement(const bn::fixed_point& position, Direction direction);
    bool attack();
    void update_attack();

    [[nodiscard]] bn::fixed movement_speed() const;
    [[nodiscard]] Direction direction() const;
    [[nodiscard]] bn::fixed_point position() const;
    [[nodiscard]] const CollisionBody& collision_body() const;

private:
    bn::sprite_ptr _sprite;
    bn::sprite_tiles_item _tiles_item;
    Direction _direction;
    bn::fixed _movement_speed;
    CollisionBody _collision_body;
    AttackBehavior& _attack_behavior;
};

#endif
