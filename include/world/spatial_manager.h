#ifndef WORLD_SPATIAL_MANAGER_H
#define WORLD_SPATIAL_MANAGER_H

#include "bn_array.h"

#include "combat/collision/collision_body.h"
#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"
#include "world/spatial_layer.h"

enum class SpatialActorId : int
{
    PLAYER,
    GOBLIN_0,
    GOBLIN_1,
    GOBLIN_2,
    GOBLIN_3,
    CROSSBOW_GOBLIN_0,
    CROSSBOW_GOBLIN_1,
    CROSSBOW_GOBLIN_2,
    CROSSBOW_GOBLIN_3,
    COUNT
};

class SpatialManager
{
public:
    static constexpr int actor_count = int(SpatialActorId::COUNT);

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

private:
    struct Actor
    {
        WorldBox pushbox = {};
        SpatialLayer layer = SpatialLayer::GROUND;
        bool active = false;
    };

    struct LayerData
    {
        const StageData* stage;
        const StageStaticObstacleData* static_obstacles;
    };

    [[nodiscard]] const LayerData& _layer_data(SpatialLayer layer) const;

    LayerData _ground;
    LayerData _upper;
    bn::array<Actor, actor_count> _actors = {};
};

#endif
