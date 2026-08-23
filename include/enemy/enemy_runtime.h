#ifndef ENEMY_ENEMY_RUNTIME_H
#define ENEMY_ENEMY_RUNTIME_H

#include "bn_array.h"
#include "bn_fixed.h"

#include "character/crossbow_goblin.h"
#include "character/goblin.h"
#include "world/spatial_manager.h"

enum class EnemyType : uint8_t
{
    NONE,
    GOBLIN,
    CROSSBOW
};

struct ActiveEnemy
{
    bool active = false;
    EnemyType type = EnemyType::NONE;
    SpatialActorId actor_id = SpatialActorId::PLAYER;
    uint8_t pool_index = 0;
};

class EnemyRuntime
{
public:
    static constexpr int goblin_count = 4;
    static constexpr int crossbow_goblin_count = 4;
    static constexpr int active_enemy_capacity = 60;

    EnemyRuntime(const bn::array<bn::fixed_point, goblin_count>& goblin_home_positions,
                 const bn::array<int, goblin_count>& goblin_target_ids,
                 const bn::array<bn::fixed_point, crossbow_goblin_count>& crossbow_home_positions,
                 const bn::array<int, crossbow_goblin_count>& crossbow_target_ids);

    bn::array<Goblin, goblin_count>& goblins();
    const bn::array<Goblin, goblin_count>& goblins() const;
    bn::array<CrossbowGoblin, crossbow_goblin_count>& crossbow_goblins();
    const bn::array<CrossbowGoblin, crossbow_goblin_count>& crossbow_goblins() const;

    ActiveEnemy& active_enemy(int index);
    const ActiveEnemy& active_enemy(int index) const;

private:
    bn::array<Goblin, goblin_count> _goblins;
    bn::array<CrossbowGoblin, crossbow_goblin_count> _crossbow_goblins;
    bn::array<ActiveEnemy, active_enemy_capacity> _active_enemies = {};
};

#endif
