#ifndef WORLD_STAGES_STAGE2_H
#define WORLD_STAGES_STAGE2_H

#include "bn_array.h"
#include "bn_fixed_point.h"

#include "world/stage_data.h"
#include "world/stage_definition.h"
#include "world/stage_static_obstacle.h"
#include "combat/collision/collision_math.h"

namespace stage2
{
    constexpr int tile_size = 8;
    constexpr int width = 20;
    constexpr int height = 20;

    [[nodiscard]] constexpr bn::array<StageCell, width * height> make_ground_movement_cells()
    {
        bn::array<StageCell, width * height> result = {};

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                bool boundary = x == 0 || x == width - 1 || y == 0 || y == height - 1;
                bool structure = (x == 3 || x == 4 || x == 15 || x == 16) &&
                                 (y == 4 || y == 5 || y == 13 || y == 14);
                result[(y * width) + x] = boundary || structure ? StageCell::BLOCKED : StageCell::WALKABLE;
            }
        }

        return result;
    }

    [[nodiscard]] constexpr bn::array<StageCell, width * height> make_upper_movement_cells()
    {
        bn::array<StageCell, width * height> result = {};

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                // These four 2x2 footprints match the raised structures in
                // generate_stage2_terrain.py.  Until stairs arrive, UPPER
                // actors remain on one of these visible platforms.
                bool upper_platform = (x == 3 || x == 4 || x == 15 || x == 16) &&
                                      (y == 4 || y == 5 || y == 13 || y == 14);
                result[(y * width) + x] = upper_platform ? StageCell::WALKABLE : StageCell::BLOCKED;
            }
        }

        return result;
    }

    constexpr bn::array<StageCell, width * height> ground_movement_cells = make_ground_movement_cells();
    constexpr bn::array<StageCell, width * height> upper_movement_cells = make_upper_movement_cells();
    constexpr StageData ground_data = { width, height, tile_size, ground_movement_cells.data() };
    constexpr StageData upper_data = { width, height, tile_size, upper_movement_cells.data() };
    // Keep the ground aliases while Stage2 is still rendered as a single visual map.
    constexpr StageData data = ground_data;
    constexpr StageStaticObstacleData ground_static_obstacles = { nullptr, 0 };
    constexpr StageStaticObstacleData upper_static_obstacles = { nullptr, 0 };
    constexpr StageStaticObstacleData static_obstacles = ground_static_obstacles;

    constexpr bn::fixed_point player_spawn(0, 56);
    constexpr StageEnemySpawn enemy_spawns[] = {
        { EnemyType::GOBLIN, { -24, -40 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 24, -40 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { -48, -40 }, SpatialLayer::UPPER },
        { EnemyType::CROSSBOW, { 48, -40 }, SpatialLayer::UPPER },
        { EnemyType::CROSSBOW, { -48, 32 }, SpatialLayer::UPPER },
        { EnemyType::CROSSBOW, { 48, 32 }, SpatialLayer::UPPER }
    };
    constexpr int enemy_spawn_count = sizeof(enemy_spawns) / sizeof(enemy_spawns[0]);
    constexpr WorldBox exit_box = { { 0, -64 }, 24, 8 };
    constexpr StageDefinition definition = {
        StageVisualId::STAGE_2,
        ground_data,
        ground_static_obstacles,
        upper_data,
        upper_static_obstacles,
        player_spawn,
        SpatialLayer::GROUND,
        enemy_spawns,
        enemy_spawn_count,
        exit_box
    };

    [[nodiscard]] constexpr bool spawn_cell_is_walkable(const StageEnemySpawn& spawn)
    {
        const StageData& stage = spawn.layer == SpatialLayer::GROUND ? ground_data : upper_data;
        int cell_x = stage_cell_from_world_coordinate(stage, int(spawn.position.x()));
        int cell_y = stage_cell_from_world_coordinate(stage, int(spawn.position.y()));
        return stage_cell_in_bounds(stage, cell_x, cell_y) &&
               stage_cell_at(stage, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr bool player_spawn_is_walkable()
    {
        int cell_x = stage_cell_from_world_coordinate(ground_data, int(player_spawn.x()));
        int cell_y = stage_cell_from_world_coordinate(ground_data, int(player_spawn.y()));
        return stage_cell_in_bounds(ground_data, cell_x, cell_y) &&
               stage_cell_at(ground_data, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr bool spawns_are_walkable()
    {
        for(int index = 0; index < enemy_spawn_count; ++index)
        {
            if(! spawn_cell_is_walkable(enemy_spawns[index]))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool same_layer_spawns_are_separated()
    {
        constexpr WorldBox player_pushbox = { { player_spawn.x(), player_spawn.y() + 1 }, 8, 8 };
        constexpr WorldBox enemy_pushboxes[enemy_spawn_count] = {
            { { enemy_spawns[0].position.x(), enemy_spawns[0].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[1].position.x(), enemy_spawns[1].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[2].position.x(), enemy_spawns[2].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[3].position.x(), enemy_spawns[3].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[4].position.x(), enemy_spawns[4].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[5].position.x(), enemy_spawns[5].position.y() + 3 }, 6, 6 }
        };

        for(int first = 0; first < enemy_spawn_count; ++first)
        {
            if(enemy_spawns[first].layer == SpatialLayer::GROUND &&
               overlaps_strictly(player_pushbox, enemy_pushboxes[first]))
            {
                return false;
            }

            for(int second = first + 1; second < enemy_spawn_count; ++second)
            {
                if(enemy_spawns[first].layer == enemy_spawns[second].layer &&
                   overlaps_strictly(enemy_pushboxes[first], enemy_pushboxes[second]))
                {
                    return false;
                }
            }
        }

        return true;
    }

    static_assert(width * height == 400);
    static_assert(stage_cell_at(ground_data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(ground_data, 10, 10) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 3, 4) == StageCell::BLOCKED);
    static_assert(stage_cell_at(ground_data, 10, 1) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 10, 17) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 7, 5) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 13, 5) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 5, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(ground_data, 15, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(upper_data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(upper_data, 3, 4) == StageCell::WALKABLE);
    static_assert(stage_cell_at(upper_data, 15, 4) == StageCell::WALKABLE);
    static_assert(stage_cell_at(upper_data, 3, 13) == StageCell::WALKABLE);
    static_assert(stage_cell_at(upper_data, 15, 13) == StageCell::WALKABLE);
    static_assert(stage_cell_at(upper_data, 5, 4) == StageCell::BLOCKED);
    static_assert(stage_cell_at(upper_data, 9, 9) == StageCell::BLOCKED);
    static_assert(stage_cell_at(ground_data, 9, 9) == StageCell::WALKABLE);
    static_assert(enemy_spawn_count == 6);
    static_assert(enemy_spawns[0].type == EnemyType::GOBLIN);
    static_assert(enemy_spawns[1].type == EnemyType::GOBLIN);
    static_assert(enemy_spawns[2].type == EnemyType::CROSSBOW);
    static_assert(enemy_spawns[5].type == EnemyType::CROSSBOW);
    static_assert(enemy_spawns[0].layer == SpatialLayer::GROUND);
    static_assert(enemy_spawns[2].layer == SpatialLayer::UPPER);
    static_assert(player_spawn_is_walkable());
    static_assert(spawn_cell_is_walkable(enemy_spawns[0]));
    static_assert(spawns_are_walkable());
    static_assert(same_layer_spawns_are_separated());
}

#endif
