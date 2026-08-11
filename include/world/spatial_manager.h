#ifndef WORLD_SPATIAL_MANAGER_H
#define WORLD_SPATIAL_MANAGER_H

#include "bn_array.h"

#include "combat/collision/collision_body.h"
#include "world/stage_data.h"
#include "world/stage_static_obstacle.h"

enum class SpatialActorId : int
{
    PLAYER,
    GOBLIN_0,
    GOBLIN_1,
    GOBLIN_2,
    GOBLIN_3,
    CROSSBOW_GOBLIN,
    COUNT
};

class SpatialManager
{
public:
    static constexpr int actor_count = int(SpatialActorId::COUNT);

    SpatialManager(const StageData& stage, const StageStaticObstacleData& static_obstacles);

    void set_actor(SpatialActorId actor_id, const WorldBox& pushbox);
    void update_actor(SpatialActorId actor_id, const WorldBox& pushbox);
    void set_actor_active(SpatialActorId actor_id, bool active);

    [[nodiscard]] const StageStaticObstacleData& static_obstacles() const;

    [[nodiscard]] WorldBoxList<max_movement_obstacles> movement_obstacles(
            SpatialActorId actor_id, const WorldBox& movement_area) const;

private:
    struct Actor
    {
        WorldBox pushbox = {};
        bool active = false;
    };

    const StageData& _stage;
    const StageStaticObstacleData& _static_obstacles;
    bn::array<Actor, actor_count> _actors = {};
};

#endif
