#include "enemy/enemy_runtime.h"

#include "bn_assert.h"

static_assert(sizeof(EnemyType) == 1);
static_assert(sizeof(ActiveEnemy) == 4);
static_assert(EnemyRuntime::active_enemy_capacity == 60);
static_assert(uint8_t(EnemyType::GOBLIN) != uint8_t(EnemyType::CROSSBOW));

EnemyRuntime::EnemyRuntime(const bn::array<bn::fixed_point, goblin_count>& goblin_home_positions,
                           const bn::array<int, goblin_count>& goblin_target_ids,
                           const bn::array<bn::fixed_point, crossbow_goblin_count>& crossbow_home_positions,
                           const bn::array<int, crossbow_goblin_count>& crossbow_target_ids) :
    _goblins{{
        Goblin(goblin_home_positions[0], goblin_target_ids[0]),
        Goblin(goblin_home_positions[1], goblin_target_ids[1]),
        Goblin(goblin_home_positions[2], goblin_target_ids[2]),
        Goblin(goblin_home_positions[3], goblin_target_ids[3])
    }},
    _crossbow_goblins{{
        CrossbowGoblin(crossbow_home_positions[0], crossbow_target_ids[0]),
        CrossbowGoblin(crossbow_home_positions[1], crossbow_target_ids[1]),
        CrossbowGoblin(crossbow_home_positions[2], crossbow_target_ids[2]),
        CrossbowGoblin(crossbow_home_positions[3], crossbow_target_ids[3])
    }}
{
}

bn::array<Goblin, EnemyRuntime::goblin_count>& EnemyRuntime::goblins()
{
    return _goblins;
}

const bn::array<Goblin, EnemyRuntime::goblin_count>& EnemyRuntime::goblins() const
{
    return _goblins;
}

bn::array<CrossbowGoblin, EnemyRuntime::crossbow_goblin_count>& EnemyRuntime::crossbow_goblins()
{
    return _crossbow_goblins;
}

const bn::array<CrossbowGoblin, EnemyRuntime::crossbow_goblin_count>& EnemyRuntime::crossbow_goblins() const
{
    return _crossbow_goblins;
}

ActiveEnemy& EnemyRuntime::active_enemy(int index)
{
    BN_ASSERT(index >= 0 && index < active_enemy_capacity);
    return _active_enemies[index];
}

const ActiveEnemy& EnemyRuntime::active_enemy(int index) const
{
    BN_ASSERT(index >= 0 && index < active_enemy_capacity);
    return _active_enemies[index];
}

void EnemyRuntime::register_enemy(EnemyType type, uint8_t pool_index, SpatialActorId actor_id)
{
    for(ActiveEnemy& enemy : _active_enemies)
    {
        if(enemy.occupied && enemy.type == type && enemy.pool_index == pool_index)
        {
            return;
        }
    }

    for(ActiveEnemy& enemy : _active_enemies)
    {
        if(! enemy.occupied)
        {
            enemy.occupied = true;
            enemy.type = type;
            enemy.pool_index = pool_index;
            enemy.actor_id = actor_id;
            return;
        }
    }

    BN_ASSERT(false, "Enemy roster full");
}

void EnemyRuntime::clear_roster()
{
    for(ActiveEnemy& enemy : _active_enemies)
    {
        enemy.occupied = false;
        enemy.type = EnemyType::NONE;
        enemy.pool_index = 0;
        enemy.actor_id = SpatialActorId::PLAYER;
    }
}

int EnemyRuntime::roster_count() const
{
    int result = 0;
    for(const ActiveEnemy& enemy : _active_enemies)
    {
        if(enemy.occupied)
        {
            ++result;
        }
    }
    return result;
}
