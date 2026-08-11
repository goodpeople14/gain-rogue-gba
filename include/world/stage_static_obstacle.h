#ifndef WORLD_STAGE_STATIC_OBSTACLE_H
#define WORLD_STAGE_STATIC_OBSTACLE_H

#include "combat/collision/collision_box.h"

struct StageStaticObstacleData
{
    const WorldBox* boxes;
    int count;
};

#endif
