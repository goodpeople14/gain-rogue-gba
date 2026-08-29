#ifndef WORLD_STAGES_STAGE5_H
#define WORLD_STAGES_STAGE5_H

#include "world/stages/stage3.h"

namespace stage5
{
    constexpr const StageData& ground_data = stage3::ground_data;
    constexpr const StageStaticObstacleData& ground_static_obstacles = stage3::ground_static_obstacles;
    constexpr const StageData& upper_data = stage3::upper_data;
    constexpr const StageStaticObstacleData& upper_static_obstacles = stage3::upper_static_obstacles;
    constexpr bn::fixed_point player_spawn = stage3::player_spawn;
    constexpr SpatialLayer player_layer = stage3::player_layer;
    constexpr WorldBox exit_box = stage3::exit_box;

    constexpr int goblin_spawn_count = 0;
    constexpr int crossbow_spawn_count = 16;
    constexpr int enemy_spawn_count = goblin_spawn_count + crossbow_spawn_count;
    constexpr StageEnemySpawn enemy_spawns[enemy_spawn_count] = {
        { EnemyType::CROSSBOW, stage3::enemy_spawns[0].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[1].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[2].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[3].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[4].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[5].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[6].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[7].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[8].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[9].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[10].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[11].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[12].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[13].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[14].position, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, stage3::enemy_spawns[15].position, SpatialLayer::GROUND }
    };

    constexpr StageDefinition definition = {
        StageVisualId::STAGE_1,
        ground_data,
        ground_static_obstacles,
        upper_data,
        upper_static_obstacles,
        player_spawn,
        player_layer,
        enemy_spawns,
        enemy_spawn_count,
        exit_box
    };

    [[nodiscard]] constexpr bool spawns_match_stage3_positions()
    {
        for(int index = 0; index < enemy_spawn_count; ++index)
        {
            if(enemy_spawns[index].position != stage3::enemy_spawns[index].position ||
               enemy_spawns[index].layer != SpatialLayer::GROUND)
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool spawn_types_are_valid()
    {
        for(const StageEnemySpawn& spawn : enemy_spawns)
        {
            if(spawn.type != EnemyType::CROSSBOW)
            {
                return false;
            }
        }

        return true;
    }

    static_assert(enemy_spawn_count == 16);
    static_assert(goblin_spawn_count == 0);
    static_assert(crossbow_spawn_count == 16);
    static_assert(spawn_types_are_valid());
    static_assert(spawns_match_stage3_positions());
    static_assert(stage3::spawns_are_valid());
}

#endif
