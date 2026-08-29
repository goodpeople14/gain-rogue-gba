#include "enemy/enemy_runtime.h"

#include "bn_assert.h"

namespace
{
    constexpr bn::fixed_point initial_home_position(0, 0);
    constexpr int goblin_target_ids[EnemyRuntime::goblin_count] = { 10, 11, 12, 13 };
    constexpr int crossbow_goblin_target_ids[EnemyRuntime::crossbow_goblin_count] = { 14, 15, 16, 17 };
    constexpr SpatialActorId goblin_actor_ids[EnemyRuntime::goblin_count] = {
        SpatialActorId::ACTOR_0, SpatialActorId::ACTOR_1,
        SpatialActorId::ACTOR_2, SpatialActorId::ACTOR_3
    };
    constexpr SpatialActorId crossbow_goblin_actor_ids[EnemyRuntime::crossbow_goblin_count] = {
        SpatialActorId::ACTOR_4, SpatialActorId::ACTOR_5,
        SpatialActorId::ACTOR_6, SpatialActorId::ACTOR_7
    };
}

static_assert(sizeof(EnemyType) == 1);
static_assert(sizeof(ActiveEnemy) == 4);
static_assert(EnemyRuntime::active_enemy_capacity == 60);
static_assert(uint8_t(EnemyType::GOBLIN) != uint8_t(EnemyType::CROSSBOW));

EnemyRuntime::EnemyRuntime() :
    _goblins{{
        Goblin(initial_home_position, goblin_target_ids[0]),
        Goblin(initial_home_position, goblin_target_ids[1]),
        Goblin(initial_home_position, goblin_target_ids[2]),
        Goblin(initial_home_position, goblin_target_ids[3])
    }},
    _crossbow_goblins{{
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[0]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[1]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[2]),
        CrossbowGoblin(initial_home_position, crossbow_goblin_target_ids[3])
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
            return register_enemy(type, uint8_t(pool_index), actor_id(type, uint8_t(pool_index)));
        }
    }

    BN_ASSERT(false, "Enemy pool exhausted");
    return _active_enemies[0];
}

ActiveEnemy& EnemyRuntime::register_enemy(EnemyType type, uint8_t pool_index, SpatialActorId actor_id)
{
    BN_ASSERT(actor_id == this->actor_id(type, pool_index), "Unexpected enemy actor id");
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

SpatialActorId EnemyRuntime::actor_id(EnemyType type, uint8_t pool_index) const
{
    switch(type)
    {
    case EnemyType::GOBLIN:
        BN_ASSERT(pool_index < goblin_count);
        return goblin_actor_ids[pool_index];
    case EnemyType::CROSSBOW:
        BN_ASSERT(pool_index < crossbow_goblin_count);
        return crossbow_goblin_actor_ids[pool_index];
    case EnemyType::NONE:
        break;
    default:
        break;
    }

    BN_ASSERT(false, "Unsupported enemy type");
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
