#ifndef COLLISION_COLLISION_BOX_H
#define COLLISION_COLLISION_BOX_H

#include "bn_fixed_point.h"

struct LocalBox
{
    int offset_x;
    int offset_y;
    int width;
    int height;
};

struct WorldBox
{
    bn::fixed_point center;
    int width;
    int height;
};

struct Hitbox
{
    LocalBox box;
};

struct Hurtbox
{
    LocalBox box;
};

struct Pushbox
{
    LocalBox box;
};

[[nodiscard]] constexpr WorldBox world_box(const bn::fixed_point& position, const LocalBox& box)
{
    return { position + bn::fixed_point(box.offset_x, box.offset_y), box.width, box.height };
}

#endif
