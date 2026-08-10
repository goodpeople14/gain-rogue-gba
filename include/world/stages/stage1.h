#ifndef WORLD_STAGES_STAGE1_H
#define WORLD_STAGES_STAGE1_H

#include "world/stage_data.h"

namespace stage1
{
    constexpr int tile_size = 8;
    constexpr int width = 20;
    constexpr int height = 20;

    constexpr StageCell movement_cells[width * height] = {
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE,
        StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::WALKABLE, StageCell::BLOCKED,

        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED,
        StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED, StageCell::BLOCKED
    };

    constexpr StageData data = { width, height, tile_size, movement_cells };

    static_assert(width * height == 400);
    static_assert(stage_cell_at(data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, width - 1, height - 1) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, 9, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 7, 6) == StageCell::BLOCKED);
    static_assert(stage_cell_from_world_coordinate(data, -80) == 0);
    static_assert(stage_cell_from_world_coordinate(data, -81) == -1);
    static_assert(stage_cell_from_world_coordinate(data, 79) == 19);
    static_assert(stage_cell_from_world_coordinate(data, 80) == 20);
}

#endif
