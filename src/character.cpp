#include "character.h"

namespace
{
    bn::fixed clamp_position(bn::fixed position, int minimum, int maximum)
    {
        if(position < minimum)
        {
            return minimum;
        }

        if(position > maximum)
        {
            return maximum;
        }

        return position;
    }
}

Character::Character(const bn::sprite_item& sprite_item, bn::fixed x, bn::fixed y,
                     Direction direction, bn::fixed movement_speed, AttackBehavior& attack_behavior) :
    _sprite(sprite_item.create_sprite(x, y, int(direction))),
    _tiles_item(sprite_item.tiles_item()),
    _direction(direction),
    _movement_speed(movement_speed),
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
}

void Character::set_visible(bool visible)
{
    _sprite.set_visible(visible);
}

void Character::move(bn::fixed delta_x, bn::fixed delta_y, Direction direction, const MovementBounds& bounds)
{
    _direction = direction;
    _sprite.set_tiles(_tiles_item, int(_direction));
    _sprite.set_position(
            clamp_position(_sprite.x() + delta_x, bounds.min_x, bounds.max_x),
            clamp_position(_sprite.y() + delta_y, bounds.min_y, bounds.max_y));
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
