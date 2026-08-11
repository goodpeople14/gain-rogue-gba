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

    [[nodiscard]] constexpr bn::array<StageCell, width * height> make_movement_cells()
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

    constexpr bn::array<StageCell, width * height> movement_cells = make_movement_cells();
    constexpr StageData data = { width, height, tile_size, movement_cells.data() };
    constexpr StageStaticObstacleData static_obstacles = { nullptr, 0 };

    constexpr bn::fixed_point player_spawn(0, 56);
    constexpr bn::array<bn::fixed_point, 4> goblin_spawns = {{
        { -24, -40 }, { 24, -40 }, { -40, -4 }, { 40, -4 }
    }};
    constexpr bn::fixed_point crossbow_spawn(0, -48);
    constexpr WorldBox exit_box = { { 0, -64 }, 24, 8 };

    static_assert(width * height == 400);
    static_assert(stage_cell_at(data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, 10, 10) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 3, 4) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, 10, 1) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 10, 17) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 7, 5) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 13, 5) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 5, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 15, 9) == StageCell::WALKABLE);
}

#endif
