#ifndef COMBAT_ATTACK_CONTEXT_H
#define COMBAT_ATTACK_CONTEXT_H

#include "bn_fixed_point.h"

#include "game/direction.h"

struct AttackContext
{
    bn::fixed_point position;
    Direction direction;
};

#endif
