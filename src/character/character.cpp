#include "character/character.h"

Character::Character(const bn::sprite_item& sprite_item, bn::fixed x, bn::fixed y,
                     Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body,
                     AttackBehavior& attack_behavior) :
    _sprite(sprite_item.create_sprite(x, y, int(direction))),
    _tiles_item(sprite_item.tiles_item()),
    _direction(direction),
    _movement_speed(movement_speed),
    _collision_body(collision_body),
    _attack_behavior(attack_behavior)
{
}

bool Character::attack()
{
    return _attack_behavior.try_attack({ _sprite.position(), _direction });
}

void Character::update_attack()
{
    _attack_behavior.update();
    _sprite.set_visible(! _attack_behavior.hides_character());
}

void Character::set_visible(bool visible)
{
    _sprite.set_visible(visible);
}

void Character::apply_movement(const bn::fixed_point& position, Direction direction)
{
    _direction = direction;
    _sprite.set_tiles(_tiles_item, int(_direction));
    _sprite.set_position(position);
}

bn::fixed Character::movement_speed() const
{
    return _movement_speed;
}

Direction Character::direction() const
{
    return _direction;
}

bn::fixed_point Character::position() const
{
    return _sprite.position();
}

const CollisionBody& Character::collision_body() const
{
    return _collision_body;
}
