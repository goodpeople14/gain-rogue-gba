#ifndef WORLD_STAGES_STAGE2_H
#define WORLD_STAGES_STAGE2_H

#include "bn_array.h"
#include "bn_fixed_point.h"

#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"

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
                bool boundary = x == 0 || x == width - 1 || y == 0 || y == height - 1;
                bool central_platform = (x == 9 || x == 10) && (y == 9 || y == 10);
                result[(y * width) + x] = boundary || central_platform ?
                        StageCell::BLOCKED : StageCell::WALKABLE;
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
    constexpr bn::array<bn::fixed_point, 4> goblin_spawns = {{
        { -24, -40 }, { 24, -40 }, { -40, -4 }, { 40, -4 }
    }};
    constexpr bn::fixed_point crossbow_spawn(0, -48);
    constexpr WorldBox exit_box = { { 0, -64 }, 24, 8 };

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
    static_assert(stage_cell_at(upper_data, 9, 9) == StageCell::BLOCKED);
    static_assert(stage_cell_at(ground_data, 9, 9) == StageCell::WALKABLE);
}

#endif
