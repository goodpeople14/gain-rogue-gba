#ifndef WORLD_SPATIAL_MANAGER_H
#define WORLD_SPATIAL_MANAGER_H

#include <stdint.h>

#include "bn_array.h"

#include "combat/collision/collision_body.h"
#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"
#include "world/spatial_layer.h"

enum class SpatialActorId : uint8_t
{
    PLAYER,
    ACTOR_0,
    ACTOR_1,
    ACTOR_2,
    ACTOR_3,
    ACTOR_4,
    ACTOR_5,
    ACTOR_6,
    ACTOR_7,
    ACTOR_8,
    ACTOR_9,
    ACTOR_10,
    ACTOR_11,
    ACTOR_12,
    ACTOR_13,
    ACTOR_14,
    ACTOR_15,
    COUNT
};

class SpatialManager
{
public:
    static constexpr int actor_count = int(SpatialActorId::COUNT);
    static constexpr int max_stage_cells = 20 * 20;
    static constexpr int cell_actor_capacity = actor_count;

    SpatialManager(const StageData& ground_stage, const StageStaticObstacleData& ground_static_obstacles,
                   const StageData& upper_stage, const StageStaticObstacleData& upper_static_obstacles);

    void set_stage(const StageData& ground_stage, const StageStaticObstacleData& ground_static_obstacles,
                   const StageData& upper_stage, const StageStaticObstacleData& upper_static_obstacles);

    void set_actor(SpatialActorId actor_id, const WorldBox& pushbox, SpatialLayer layer);
    void update_actor(SpatialActorId actor_id, const WorldBox& pushbox, SpatialLayer layer);
    void set_actor_active(SpatialActorId actor_id, bool active);

    [[nodiscard]] const StageStaticObstacleData& static_obstacles(SpatialLayer layer) const;

    [[nodiscard]] WorldBoxList<max_movement_obstacles> movement_obstacles(
            SpatialActorId actor_id, const WorldBox& movement_area) const;

#if defined(GAIN_DEBUG_LOGS)
    void validate_position_table() const;
#endif

private:
    struct CellActorList
    {
        bn::array<SpatialActorId, cell_actor_capacity> actor_ids = {};
        uint8_t count = 0;
    };

    struct Actor
    {
        WorldBox pushbox = {};
        SpatialLayer layer = SpatialLayer::GROUND;
        bool active = false;
        bool registered = false;
    };

    struct LayerData
    {
        const StageData* stage;
        const StageStaticObstacleData* static_obstacles;
    };

    struct PositionCellRange
    {
        int min_x;
        int max_x;
        int min_y;
        int max_y;
    };

    [[nodiscard]] const LayerData& _layer_data(SpatialLayer layer) const;
    [[nodiscard]] PositionCellRange _stage_cell_range(const WorldBox& area) const;
    void _clear_position_table();
    void _remove_actor_from_position_table(SpatialActorId actor_id, const WorldBox& pushbox);
    void _register_actor_in_position_table(SpatialActorId actor_id, const WorldBox& pushbox);

#if defined(GAIN_DEBUG_LOGS)
    void _validate_position_table_is_empty() const;
#endif

    LayerData _ground;
    LayerData _upper;
    bn::array<Actor, actor_count> _actors = {};
    bn::array<CellActorList, max_stage_cells> _position_table = {};
    int _stage_cell_count = 0;
};

#endif
