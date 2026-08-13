#include "world/spatial_manager.h"

#include "bn_assert.h"

#include "combat/collision/collision_math.h"

namespace
{
    [[nodiscard]] constexpr int actor_index(SpatialActorId actor_id)
    {
        return int(actor_id);
    }

    [[nodiscard]] constexpr bool same_spatial_layer(SpatialLayer first, SpatialLayer second)
    {
        return first == second;
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
                                                       bool candidate_active, SpatialLayer requesting_layer,
                                                       SpatialLayer candidate_layer)
    {
        return candidate_active && requesting_actor != candidate_actor &&
               same_spatial_layer(requesting_layer, candidate_layer);
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
    static_assert(! should_include_actor(SpatialActorId::PLAYER, SpatialActorId::PLAYER, true,
                                        SpatialLayer::GROUND, SpatialLayer::GROUND));
    static_assert(! should_include_actor(SpatialActorId::PLAYER, SpatialActorId::GOBLIN_0, false,
                                        SpatialLayer::GROUND, SpatialLayer::GROUND));
    static_assert(should_include_actor(SpatialActorId::PLAYER, SpatialActorId::GOBLIN_0, true,
                                       SpatialLayer::GROUND, SpatialLayer::GROUND));
    static_assert(! should_include_actor(SpatialActorId::PLAYER, SpatialActorId::GOBLIN_0, true,
                                        SpatialLayer::GROUND, SpatialLayer::UPPER));
}

SpatialManager::SpatialManager(const StageData& ground_stage, const StageStaticObstacleData& ground_static_obstacles,
                               const StageData& upper_stage, const StageStaticObstacleData& upper_static_obstacles) :
    _ground{ &ground_stage, &ground_static_obstacles },
    _upper{ &upper_stage, &upper_static_obstacles }
{
    set_stage(ground_stage, ground_static_obstacles, upper_stage, upper_static_obstacles);
}

void SpatialManager::set_stage(const StageData& ground_stage, const StageStaticObstacleData& ground_static_obstacles,
                               const StageData& upper_stage, const StageStaticObstacleData& upper_static_obstacles)
{
    BN_ASSERT(ground_stage.width > 0 && ground_stage.height > 0 && ground_stage.tile_size > 0);
    BN_ASSERT(upper_stage.width > 0 && upper_stage.height > 0 && upper_stage.tile_size > 0);
    BN_ASSERT(ground_stage.movement_cells && upper_stage.movement_cells);
    BN_ASSERT(ground_static_obstacles.count >= 0 && upper_static_obstacles.count >= 0);
    BN_ASSERT(ground_static_obstacles.count == 0 || ground_static_obstacles.boxes);
    BN_ASSERT(upper_static_obstacles.count == 0 || upper_static_obstacles.boxes);
    BN_ASSERT(ground_static_obstacles.count <= max_stage_object_movement_obstacles);
    BN_ASSERT(upper_static_obstacles.count <= max_stage_object_movement_obstacles);
    _ground = { &ground_stage, &ground_static_obstacles };
    _upper = { &upper_stage, &upper_static_obstacles };
}

void SpatialManager::set_actor(SpatialActorId actor_id, const WorldBox& pushbox, SpatialLayer layer)
{
    Actor& actor = _actors[actor_index(actor_id)];
    actor.pushbox = pushbox;
    actor.layer = layer;
    actor.active = true;
}

void SpatialManager::update_actor(SpatialActorId actor_id, const WorldBox& pushbox, SpatialLayer layer)
{
    Actor& actor = _actors[actor_index(actor_id)];
    actor.pushbox = pushbox;
    actor.layer = layer;
}

void SpatialManager::set_actor_active(SpatialActorId actor_id, bool active)
{
    _actors[actor_index(actor_id)].active = active;
}

const StageStaticObstacleData& SpatialManager::static_obstacles(SpatialLayer layer) const
{
    return *_layer_data(layer).static_obstacles;
}

const SpatialManager::LayerData& SpatialManager::_layer_data(SpatialLayer layer) const
{
    return layer == SpatialLayer::GROUND ? _ground : _upper;
}

WorldBoxList<max_movement_obstacles> SpatialManager::movement_obstacles(
        SpatialActorId actor_id, const WorldBox& movement_area) const
{
    WorldBoxList<max_movement_obstacles> result;

    const SpatialLayer layer = _actors[actor_index(actor_id)].layer;
    const LayerData& layer_data = _layer_data(layer);
    const StageData& stage = *layer_data.stage;
    const StageStaticObstacleData& static_obstacles = *layer_data.static_obstacles;
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
        if(should_include_actor(actor_id, candidate_id, actor.active, layer, actor.layer) &&
           touches_or_intersects(movement_area, actor.pushbox))
        {
            BN_ASSERT(result.count < max_movement_obstacles);
            result.boxes[result.count] = actor.pushbox;
            ++result.count;
        }
    }

    return result;
}
