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

    // The visual rock spans cells (7-8, 6-7).  Collision is inset by 3px on
    // every side so the rounded 16x16 drawing is not treated as a square wall.
    constexpr int rock_visual_size = 16;
    constexpr int rock_collision_inset = 3;
    constexpr int rock_collision_size = rock_visual_size - (rock_collision_inset * 2);
    constexpr bn::fixed_point rock_visual_center(-16, -24);
    constexpr int static_obstacle_count = 1;
    constexpr WorldBox static_obstacle_boxes[static_obstacle_count] = {
        { rock_visual_center, rock_collision_size, rock_collision_size }
    };
    constexpr StageStaticObstacleData static_obstacles = { static_obstacle_boxes, static_obstacle_count };

    // The exit is a passable trigger zone near the top center of the stage;
    // it deliberately does not participate in movement collision.
    constexpr WorldBox exit_box = { { 0, -64 }, 24, 8 };

    static_assert(width * height == 400);
    static_assert(stage_cell_at(data, 0, 0) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, width - 1, height - 1) == StageCell::BLOCKED);
    static_assert(stage_cell_at(data, 9, 9) == StageCell::WALKABLE);
    static_assert(stage_cell_at(data, 7, 6) == StageCell::WALKABLE);
    static_assert(rock_visual_size == 16);
    static_assert(rock_collision_size == 10);
    static_assert(static_obstacle_boxes[0].center == rock_visual_center);
    static_assert(static_obstacle_boxes[0].width == rock_collision_size &&
                  static_obstacle_boxes[0].height == rock_collision_size);
    static_assert((rock_visual_size - static_obstacle_boxes[0].width) / 2 == rock_collision_inset);
    static_assert((rock_visual_size - static_obstacle_boxes[0].height) / 2 == rock_collision_inset);
    static_assert(exit_box.center.x() == 0 && exit_box.center.y() == -64);
    static_assert(stage_cell_from_world_coordinate(data, -80) == 0);
    static_assert(stage_cell_from_world_coordinate(data, -81) == -1);
    static_assert(stage_cell_from_world_coordinate(data, 79) == 19);
    static_assert(stage_cell_from_world_coordinate(data, 80) == 20);
}

#endif
