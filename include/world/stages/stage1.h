#ifndef WORLD_STAGES_STAGE1_H
#define WORLD_STAGES_STAGE1_H

#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"
#include "world/stage_definition.h"
#include "combat/collision/collision_math.h"

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

    constexpr bn::fixed_point player_spawn(0, 56);
    constexpr StageEnemySpawn enemy_spawns[] = {
        { EnemyType::GOBLIN, { -48, -40 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 48, -40 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { -40, -4 }, SpatialLayer::GROUND },
        { EnemyType::GOBLIN, { 40, -4 }, SpatialLayer::GROUND },
        { EnemyType::CROSSBOW, { 0, -48 }, SpatialLayer::GROUND }
    };
    constexpr int enemy_spawn_count = sizeof(enemy_spawns) / sizeof(enemy_spawns[0]);
    constexpr StageDefinition definition = {
        StageVisualId::STAGE_1,
        data,
        static_obstacles,
        data,
        static_obstacles,
        player_spawn,
        SpatialLayer::GROUND,
        enemy_spawns,
        enemy_spawn_count,
        exit_box
    };

    [[nodiscard]] constexpr bool spawn_cell_is_walkable(const bn::fixed_point& position)
    {
        int cell_x = stage_cell_from_world_coordinate(data, int(position.x()));
        int cell_y = stage_cell_from_world_coordinate(data, int(position.y()));
        return stage_cell_in_bounds(data, cell_x, cell_y) &&
               stage_cell_at(data, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr bool spawns_are_walkable()
    {
        for(int index = 0; index < enemy_spawn_count; ++index)
        {
            if(! spawn_cell_is_walkable(enemy_spawns[index].position))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool spawns_are_separated()
    {
        constexpr WorldBox player_pushbox = { { player_spawn.x(), player_spawn.y() + 1 }, 8, 8 };
        constexpr WorldBox enemy_pushboxes[enemy_spawn_count] = {
            { { enemy_spawns[0].position.x(), enemy_spawns[0].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[1].position.x(), enemy_spawns[1].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[2].position.x(), enemy_spawns[2].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[3].position.x(), enemy_spawns[3].position.y() + 3 }, 6, 6 },
            { { enemy_spawns[4].position.x(), enemy_spawns[4].position.y() + 3 }, 6, 6 }
        };

        for(int first = 0; first < enemy_spawn_count; ++first)
        {
            if(overlaps_strictly(player_pushbox, enemy_pushboxes[first]) ||
               overlaps_strictly(enemy_pushboxes[first], static_obstacle_boxes[0]))
            {
                return false;
            }

            for(int second = first + 1; second < enemy_spawn_count; ++second)
            {
                if(overlaps_strictly(enemy_pushboxes[first], enemy_pushboxes[second]))
                {
                    return false;
                }
            }
        }

        return true;
    }

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
    static_assert(enemy_spawn_count == 5);
    static_assert(enemy_spawns[0].type == EnemyType::GOBLIN);
    static_assert(enemy_spawns[4].type == EnemyType::CROSSBOW);
    static_assert(spawn_cell_is_walkable(player_spawn));
    static_assert(spawns_are_walkable());
    static_assert(spawns_are_separated());
    static_assert(stage_cell_from_world_coordinate(data, -80) == 0);
    static_assert(stage_cell_from_world_coordinate(data, -81) == -1);
    static_assert(stage_cell_from_world_coordinate(data, 79) == 19);
    static_assert(stage_cell_from_world_coordinate(data, 80) == 20);
}

#endif
