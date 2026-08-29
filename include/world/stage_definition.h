#ifndef WORLD_STAGE_DEFINITION_H
#define WORLD_STAGE_DEFINITION_H

#include <stdint.h>

#include "bn_fixed_point.h"

#include "enemy/enemy_type.h"
#include "combat/collision/collision_box.h"
#include "world/spatial_layer.h"
#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"

enum class StageId : uint8_t
{
    STAGE_1,
    STAGE_2,
    STAGE_3,
    STAGE_4,
    STAGE_5
};

enum class StageVisualId : uint8_t
{
    STAGE_1,
    STAGE_2
};

struct StageEnemySpawn
{
    EnemyType type;
    bn::fixed_point position;
    SpatialLayer layer;
};

struct StageDefinition
{
    StageVisualId visual;
    const StageData& ground_stage;
    const StageStaticObstacleData& ground_static_obstacles;
    const StageData& upper_stage;
    const StageStaticObstacleData& upper_static_obstacles;
    bn::fixed_point player_spawn;
    SpatialLayer player_layer;
    const StageEnemySpawn* enemy_spawns;
    int enemy_spawn_count;
    WorldBox exit_box;
};

[[nodiscard]] const StageDefinition& stage_definition(StageId stage);

#endif
