#ifndef COLLISION_COLLISION_BODY_H
#define COLLISION_COLLISION_BODY_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

struct CollisionBody
{
    Hurtbox hurtbox;
    Pushbox pushbox;
};

// A query no larger than 12x12 can touch at most a 3x3 group of 8x8 Stage cells.
constexpr int max_static_movement_obstacles = 9;
// A moving actor can be blocked by every other current actor.
constexpr int max_dynamic_movement_obstacles = 5;
constexpr int max_movement_obstacles = max_static_movement_obstacles + max_dynamic_movement_obstacles;

template<int Capacity>
struct WorldBoxList
{
    bn::array<WorldBox, Capacity> boxes = {};
    int count = 0;
};

#endif
