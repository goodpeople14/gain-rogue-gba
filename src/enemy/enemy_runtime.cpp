#include "enemy/enemy_runtime.h"

#include "bn_assert.h"

namespace
{
    constexpr bn::fixed_point initial_home_position(0, 0);
    constexpr int goblin_target_ids[EnemyRuntime::goblin_count] = {
        10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25
    };
    constexpr int crossbow_goblin_target_ids[EnemyRuntime::crossbow_goblin_count] = {
        26, 27, 28, 29, 30, 31, 32, 33,
        34, 35, 36, 37, 38, 39, 40, 41
    };
}

static_assert(sizeof(EnemyType) == 1);
static_assert(sizeof(ActiveEnemy) == 4);
static_assert(EnemyRuntime::active_enemy_capacity == 60);
static_assert(EnemyRuntime::goblin_count == 16);
static_assert(EnemyRuntime::crossbow_goblin_count == 16);
static_assert(SpatialManager::actor_count == 17);
static_assert(EnemyRuntime::goblin_count >= SpatialManager::actor_count - 1);
static_assert(EnemyRuntime::crossbow_goblin_count >= SpatialManager::actor_count - 1);
static_assert(uint8_t(EnemyType::GOBLIN) != uint8_t(EnemyType::CROSSBOW));

EnemyRuntime::EnemyRuntime() :
    _goblins{{
        Goblin(initial_home_position, goblin_target_ids[0]),
        Goblin(initial_home_position, goblin_target_ids[1]),
        Goblin(initial_home_position, goblin_target_ids[2]),
        Goblin(initial_home_position, goblin_target_ids[3]),
        Goblin(initial_home_position, goblin_target_ids[4]),
        Goblin(initial_home_position, goblin_target_ids[5]),
        Goblin(initial_home_position, goblin_target_ids[6]),
        Goblin(initial_home_position, goblin_target_ids[7]),
        Goblin(initial_home_position, goblin_target_ids[8]),
        Goblin(initial_home_position, goblin_target_ids[9]),
        Goblin(initial_home_position, goblin_target_ids[10]),
        Goblin(initial_home_position, goblin_target_ids[11]),
        Goblin(initial_home_position, goblin_target_ids[12]),
        Goblin(initial_home_position, goblin_target_ids[13]),
        Goblin(initial_home_position, goblin_target_ids[14]),
        Goblin(initial_home_position, goblin_target_ids[15])
    }},
    _crossbow_goblins{{
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[0]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[1]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[2]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[3]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[4]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[5]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[6]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[7]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[8]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[9]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[10]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[11]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[12]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[13]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[14]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[15])
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

ActiveEnemy& EnemyRuntime::allocate_enemy(EnemyType type)
{
    const int pool_count = type == EnemyType::GOBLIN ? goblin_count :
                           type == EnemyType::CROSSBOW ? crossbow_goblin_count : 0;
    BN_ASSERT(pool_count > 0, "Unsupported enemy type");

    for(int pool_index = 0; pool_index < pool_count; ++pool_index)
    {
        bool allocated = false;
        for(const ActiveEnemy& enemy : _active_enemies)
        {
            allocated |= enemy.occupied && enemy.type == type && enemy.pool_index == pool_index;
        }
        if(! allocated)
        {
            return _register_enemy(type, uint8_t(pool_index), _allocate_actor_id());
        }
    }

    BN_ASSERT(false, "Enemy pool exhausted");
    return _active_enemies[0];
}

ActiveEnemy& EnemyRuntime::_register_enemy(EnemyType type, uint8_t pool_index, SpatialActorId actor_id)
{
    for(ActiveEnemy& enemy : _active_enemies)
    {
        if(enemy.occupied && enemy.type == type && enemy.pool_index == pool_index)
        {
            return enemy;
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
            return enemy;
        }
    }

    BN_ASSERT(false, "Enemy roster full");
    return _active_enemies[0];
}

SpatialActorId EnemyRuntime::_allocate_actor_id() const
{
    for(int index = int(SpatialActorId::ACTOR_0); index < SpatialManager::actor_count; ++index)
    {
        const SpatialActorId actor_id = SpatialActorId(index);
        bool allocated = false;
        for(const ActiveEnemy& enemy : _active_enemies)
        {
            allocated |= enemy.occupied && enemy.actor_id == actor_id;
        }
        if(! allocated)
        {
            return actor_id;
        }
    }

    BN_ASSERT(false, "Spatial actor slots exhausted");
    return SpatialActorId::PLAYER;
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
