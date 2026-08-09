#ifndef COLLISION_COLLISION_BODY_H
#define COLLISION_COLLISION_BODY_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

struct CollisionBody
{
    Hurtbox hurtbox;
    Pushbox pushbox;
};

constexpr int max_movement_obstacles = 4;

template<int Capacity>
struct WorldBoxList
{
    bn::array<WorldBox, Capacity> boxes = {};
    int count = 0;
};

#endif
