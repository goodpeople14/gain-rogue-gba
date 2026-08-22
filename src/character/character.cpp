#include "character/character.h"

Character::Character(const bn::sprite_item& sprite_item, const bn::fixed_point& initial_position,
                     Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body,
                     const CharacterDefinition& definition) :
    _sprite(sprite_item.create_sprite(initial_position, int(direction))),
    _tiles_item(sprite_item.tiles_item()),
    _direction(direction),
    _movement_speed(movement_speed),
    _collision_body(collision_body),
    _definition(&definition),
    _health(definition.max_hp)
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

void Character::take_damage(int damage)
{
    _health.damage(damage);
}

void Character::reset_health()
{
    _health.reset();
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

bn::fixed_point Character::foot_position() const
{
    return world_foot_position(position(), _collision_body.pushbox);
}

const CollisionBody& Character::collision_body() const
{
    return _collision_body;
}

SpatialLayer Character::spatial_layer() const
{
    return _spatial_layer;
}

const CharacterDefinition& Character::definition() const
{
    return *_definition;
}

int Character::current_health() const
{
    return _health.current();
}

int Character::max_health() const
{
    return _health.max();
}

bool Character::dead() const
{
    return _health.is_dead();
}
