#ifndef CHARACTER_MOVEMENT_INTENT_H
#define CHARACTER_MOVEMENT_INTENT_H

#include "bn_fixed_point.h"

#include "game/direction.h"

struct MovementIntent
{
    bn::fixed_point delta;
    Direction direction;
    bool moving;
};

#endif
