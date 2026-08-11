#include "world/spatial_manager.h"

#include "bn_assert.h"

#include "combat/collision/collision_math.h"

namespace
{
    [[nodiscard]] constexpr int actor_index(SpatialActorId actor_id)
    {
        return int(actor_id);
    }

    [[nodiscard]] constexpr WorldBox stage_cell_world_box(const StageData& stage, int cell_x, int cell_y)
    {
        int world_minimum = stage_world_minimum(stage);
        return {
            { world_minimum + (cell_x * stage.tile_size) + (stage.tile_size / 2),
              world_minimum + (cell_y * stage.tile_size) + (stage.tile_size / 2) },
            stage.tile_size, stage.tile_size
        };
    }

    [[nodiscard]] constexpr bool should_include_actor(SpatialActorId requesting_actor, SpatialActorId candidate_actor,
                                                       bool candidate_active)
    {
        return candidate_active && requesting_actor != candidate_actor;
    }

    constexpr StageCell spatial_test_cells[4] = {
        StageCell::WALKABLE, StageCell::BLOCKED,
        StageCell::WALKABLE, StageCell::WALKABLE
    };
    constexpr StageData spatial_test_stage = { 2, 2, 8, spatial_test_cells };
    constexpr WorldBox spatial_test_obstacle_boxes[1] = {{ { 0, 0 }, 12, 12 }};
    constexpr StageStaticObstacleData spatial_test_obstacles = { spatial_test_obstacle_boxes, 1 };

    static_assert(stage_world_minimum(spatial_test_stage) == -8);
    static_assert(stage_cell_from_world_coordinate(spatial_test_stage, -8) == 0);
    static_assert(stage_cell_from_world_coordinate(spatial_test_stage, -9) == -1);
    static_assert(stage_cell_from_world_coordinate(spatial_test_stage, -1) == 0);
    static_assert(stage_cell_from_world_coordinate(spatial_test_stage, 0) == 1);
    static_assert(stage_cell_from_world_coordinate(spatial_test_stage, 7) == 1);
    static_assert(stage_cell_world_box(spatial_test_stage, 0, 0).center == bn::fixed_point(-4, -4));
    static_assert(stage_cell_world_box(spatial_test_stage, 1, 0).center == bn::fixed_point(4, -4));
    static_assert(spatial_test_obstacles.count == 1);
    static_assert(touches_or_intersects({ { 6, 0 }, 8, 8 }, spatial_test_obstacle_boxes[0]));
    static_assert(! should_include_actor(SpatialActorId::PLAYER, SpatialActorId::PLAYER, true));
    static_assert(! should_include_actor(SpatialActorId::PLAYER, SpatialActorId::GOBLIN_0, false));
    static_assert(should_include_actor(SpatialActorId::PLAYER, SpatialActorId::GOBLIN_0, true));
}

SpatialManager::SpatialManager(const StageData& stage, const StageStaticObstacleData& static_obstacles) :
    _stage(&stage),
    _static_obstacles(&static_obstacles)
{
    set_stage(stage, static_obstacles);
}

void SpatialManager::set_stage(const StageData& stage, const StageStaticObstacleData& static_obstacles)
{
    BN_ASSERT(stage.width > 0);
    BN_ASSERT(stage.height > 0);
    BN_ASSERT(stage.tile_size > 0);
    BN_ASSERT(stage.movement_cells);
    BN_ASSERT(static_obstacles.count >= 0);
    BN_ASSERT(static_obstacles.count == 0 || static_obstacles.boxes);
    BN_ASSERT(static_obstacles.count <= max_stage_object_movement_obstacles);
    _stage = &stage;
    _static_obstacles = &static_obstacles;
}

void SpatialManager::set_actor(SpatialActorId actor_id, const WorldBox& pushbox)
{
    Actor& actor = _actors[actor_index(actor_id)];
    actor.pushbox = pushbox;
    actor.active = true;
}

void SpatialManager::update_actor(SpatialActorId actor_id, const WorldBox& pushbox)
{
    _actors[actor_index(actor_id)].pushbox = pushbox;
}

void SpatialManager::set_actor_active(SpatialActorId actor_id, bool active)
{
    _actors[actor_index(actor_id)].active = active;
}

const StageStaticObstacleData& SpatialManager::static_obstacles() const
{
    return *_static_obstacles;
}

WorldBoxList<max_movement_obstacles> SpatialManager::movement_obstacles(
        SpatialActorId actor_id, const WorldBox& movement_area) const
{
    WorldBoxList<max_movement_obstacles> result;

    const StageData& stage = *_stage;
    const StageStaticObstacleData& static_obstacles = *_static_obstacles;
    for(int cell_y = 0; cell_y < stage.height; ++cell_y)
    {
        for(int cell_x = 0; cell_x < stage.width; ++cell_x)
        {
            if(stage_cell_at(stage, cell_x, cell_y) != StageCell::BLOCKED)
            {
                continue;
            }

            WorldBox cell = stage_cell_world_box(stage, cell_x, cell_y);
            if(touches_or_intersects(movement_area, cell))
            {
                BN_ASSERT(result.count < max_movement_obstacles);
                result.boxes[result.count] = cell;
                ++result.count;
            }
        }
    }

    for(int index = 0; index < static_obstacles.count; ++index)
    {
        const WorldBox& obstacle = static_obstacles.boxes[index];
        if(touches_or_intersects(movement_area, obstacle))
        {
            BN_ASSERT(result.count < max_movement_obstacles);
            result.boxes[result.count] = obstacle;
            ++result.count;
        }
    }

    for(int index = 0; index < actor_count; ++index)
    {
        SpatialActorId candidate_id = SpatialActorId(index);
        const Actor& actor = _actors[index];
        if(should_include_actor(actor_id, candidate_id, actor.active) &&
           touches_or_intersects(movement_area, actor.pushbox))
        {
            BN_ASSERT(result.count < max_movement_obstacles);
            result.boxes[result.count] = actor.pushbox;
            ++result.count;
        }
    }

    return result;
}
