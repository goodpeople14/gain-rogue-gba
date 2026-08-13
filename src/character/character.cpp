#include "character/character.h"

Character::Character(const bn::sprite_item& sprite_item, const bn::fixed_point& initial_position,
                     Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body) :
    _sprite(sprite_item.create_sprite(initial_position, int(direction))),
    _tiles_item(sprite_item.tiles_item()),
    _direction(direction),
    _movement_speed(movement_speed),
    _collision_body(collision_body)
{
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

void Character::set_spatial_layer(SpatialLayer layer)
{
    _spatial_layer = layer;
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

SpatialLayer Character::spatial_layer() const
{
    return _spatial_layer;
}
