#ifndef COLLISION_COLLISION_BODY_H
#define COLLISION_COLLISION_BODY_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

struct CollisionBody
{
    Hurtbox hurtbox;
    Pushbox pushbox;
};

// One moving enemy can be blocked by the player plus the four other enemies.
constexpr int max_movement_obstacles = 5;

template<int Capacity>
struct WorldBoxList
{
    bn::array<WorldBox, Capacity> boxes = {};
    int count = 0;
};

#endif
