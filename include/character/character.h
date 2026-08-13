#ifndef CHARACTER_H
#define CHARACTER_H

#include "bn_fixed.h"
#include "bn_fixed_point.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"

#include "combat/collision/collision_body.h"
#include "game/direction.h"
#include "world/spatial_layer.h"

class Character
{
public:
    void set_visible(bool visible);
    void apply_movement(const bn::fixed_point& position, Direction direction);
    void set_spatial_layer(SpatialLayer layer);

    [[nodiscard]] bn::fixed movement_speed() const;
    [[nodiscard]] Direction direction() const;
    [[nodiscard]] bn::fixed_point position() const;
    [[nodiscard]] const CollisionBody& collision_body() const;
    [[nodiscard]] SpatialLayer spatial_layer() const;

protected:
    Character(const bn::sprite_item& sprite_item, const bn::fixed_point& initial_position,
              Direction direction, bn::fixed movement_speed, const CollisionBody& collision_body);

private:
    bn::sprite_ptr _sprite;
    bn::sprite_tiles_item _tiles_item;
    Direction _direction;
    bn::fixed _movement_speed;
    CollisionBody _collision_body;
    SpatialLayer _spatial_layer = SpatialLayer::GROUND;
};

#endif
