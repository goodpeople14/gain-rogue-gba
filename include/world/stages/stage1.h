#ifndef WORLD_STAGES_STAGE1_H
#define WORLD_STAGES_STAGE1_H

#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"

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

    // The visual rock spans cells (7-8, 6-7), centered at (-16, -24).  Its
    // smaller box avoids treating a rounded 16x16 drawing as a square wall.
    constexpr int static_obstacle_count = 1;
    constexpr WorldBox static_obstacle_boxes[static_obstacle_count] = {
        { { -16, -24 }, 12, 12 }
    };
    constexpr StageStaticObstacleData static_obstacles = { static_obstacle_boxes, static_obstacle_count };

    static_assert(width * height == 400);
    static_assert(stage_cell_at(data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, width - 1, height - 1) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, 9, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 7, 6) == StageCell::WALKABLE);
    static_assert(static_obstacle_boxes[0].center == bn::fixed_point(-16, -24));
    static_assert(static_obstacle_boxes[0].width == 12 && static_obstacle_boxes[0].height == 12);
    static_assert(stage_cell_from_world_coordinate(data, -80) == 0);
    static_assert(stage_cell_from_world_coordinate(data, -81) == -1);
    static_assert(stage_cell_from_world_coordinate(data, 79) == 19);
    static_assert(stage_cell_from_world_coordinate(data, 80) == 20);
}

#endif
