#ifndef ENEMY_ENEMY_RUNTIME_H
#define ENEMY_ENEMY_RUNTIME_H

#include "bn_array.h"
#include "bn_fixed.h"

#include "character/crossbow_goblin.h"
#include "character/goblin.h"
#include "enemy/enemy_type.h"
#include "world/spatial_manager.h"

struct ActiveEnemy
{
    bool occupied = false;
    EnemyType type = EnemyType::NONE;
    SpatialActorId actor_id = SpatialActorId::PLAYER;
    uint8_t pool_index = 0;
};

class EnemyRuntime
{
public:
    static constexpr int goblin_count = 16;
    static constexpr int crossbow_goblin_count = 16;
    static constexpr int active_enemy_capacity = 60;

    EnemyRuntime();

    bn::array<Goblin, goblin_count>& goblins();
    const bn::array<Goblin, goblin_count>& goblins() const;
    bn::array<CrossbowGoblin, crossbow_goblin_count>& crossbow_goblins();
    const bn::array<CrossbowGoblin, crossbow_goblin_count>& crossbow_goblins() const;

    ActiveEnemy& active_enemy(int index);
    const ActiveEnemy& active_enemy(int index) const;
    ActiveEnemy& allocate_enemy(EnemyType type);
    void clear_roster();
    [[nodiscard]] int roster_count() const;

private:
    ActiveEnemy& _register_enemy(EnemyType type, uint8_t pool_index, SpatialActorId actor_id);
    [[nodiscard]] SpatialActorId _allocate_actor_id() const;
    bn::array<Goblin, goblin_count> _goblins;
    bn::array<CrossbowGoblin, crossbow_goblin_count> _crossbow_goblins;
    bn::array<ActiveEnemy, active_enemy_capacity> _active_enemies = {};
};

#endif
