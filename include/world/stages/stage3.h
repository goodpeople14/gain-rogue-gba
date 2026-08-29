#ifndef WORLD_STAGES_STAGE3_H
#define WORLD_STAGES_STAGE3_H

#include "combat/collision/collision_math.h"
#include "world/stages/stage1.h"

namespace stage3
{
    constexpr const StageData& ground_data = stage1::data;
    constexpr const StageStaticObstacleData& ground_static_obstacles = stage1::static_obstacles;
    constexpr const StageData& upper_data = stage1::data;
    constexpr const StageStaticObstacleData& upper_static_obstacles = stage1::static_obstacles;
    constexpr bn::fixed_point player_spawn = stage1::player_spawn;
    constexpr SpatialLayer player_layer = SpatialLayer::GROUND;
    constexpr WorldBox exit_box = stage1::exit_box;

    constexpr int goblin_spawn_count = 8;
    constexpr int crossbow_spawn_count = 8;
    constexpr int enemy_spawn_count = goblin_spawn_count + crossbow_spawn_count;
    constexpr StageEnemySpawn enemy_spawns[enemy_spawn_count] = {
        { EnemyType::GOBLIN, { -56, -48 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { -24, -48 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 24, -48 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 56, -48 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { -56, -8 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { -40, 24 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 40, 24 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 56, -8 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { -48, -24 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 16, -24 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 48, -24 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { -56, 48 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { -24, 40 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 24, 40 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 56, 48 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 0, 16 }, SpatialLayer::GROUND }
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

    [[nodiscard]] constexpr bool spawn_cell_is_walkable(const StageEnemySpawn& spawn)
    {
        int cell_x = stage_cell_from_world_coordinate(ground_data, int(spawn.position.x()));
        int cell_y = stage_cell_from_world_coordinate(ground_data, int(spawn.position.y()));
        return stage_cell_in_bounds(ground_data, cell_x, cell_y) &&
               stage_cell_at(ground_data, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr WorldBox enemy_pushbox(const StageEnemySpawn& spawn)
    {
        return { { spawn.position.x(), spawn.position.y() + 3 }, 6, 6 };
    }

    [[nodiscard]] constexpr bool spawns_are_valid()
    {
        constexpr WorldBox player_pushbox = { { player_spawn.x(), player_spawn.y() + 1 }, 8, 8 };
        for(int first = 0; first < enemy_spawn_count; ++first)
        {
            const StageEnemySpawn& first_spawn = enemy_spawns[first];
            WorldBox first_pushbox = enemy_pushbox(first_spawn);
            if(! spawn_cell_is_walkable(first_spawn) ||
               overlaps_strictly(player_pushbox, first_pushbox) ||
               overlaps_strictly(exit_box, first_pushbox))
            {
                return false;
            }

            for(int obstacle_index = 0; obstacle_index < ground_static_obstacles.count; ++obstacle_index)
            {
                if(overlaps_strictly(ground_static_obstacles.boxes[obstacle_index], first_pushbox))
                {
                    return false;
                }
            }

            for(int second = first + 1; second < enemy_spawn_count; ++second)
            {
                const StageEnemySpawn& second_spawn = enemy_spawns[second];
                if(first_spawn.layer == second_spawn.layer &&
                   overlaps_strictly(first_pushbox, enemy_pushbox(second_spawn)))
                {
                    return false;
                }
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool spawn_types_are_valid()
    {
        for(int index = 0; index < goblin_spawn_count; ++index)
        {
            if(enemy_spawns[index].type != EnemyType::GOBLIN)
            {
                return false;
            }
        }

        for(int index = goblin_spawn_count; index < enemy_spawn_count; ++index)
        {
            if(enemy_spawns[index].type != EnemyType::CROSSBOW)
            {
                return false;
            }
        }

        return true;
    }

    static_assert(ground_data.width == stage1::data.width && ground_data.height == stage1::data.height);
    static_assert(ground_data.movement_cells == stage1::data.movement_cells);
    static_assert(ground_static_obstacles.boxes == stage1::static_obstacles.boxes);
    static_assert(player_spawn == stage1::player_spawn);
    static_assert(exit_box.center == stage1::exit_box.center);
    static_assert(definition.visual == StageVisualId::STAGE_1);
    static_assert(goblin_spawn_count == 8);
    static_assert(crossbow_spawn_count == 8);
    static_assert(enemy_spawn_count == 16);
    static_assert(spawn_types_are_valid());
    static_assert(spawns_are_valid());
}

#endif
