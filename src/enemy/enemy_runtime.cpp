#include "enemy/enemy_runtime.h"

#include "bn_assert.h"

namespace
{
    constexpr bn::fixed_point initial_home_position(0, 0);
    constexpr int first_enemy_target_id = 10;
}

static_assert(sizeof(EnemyType) == 1);
static_assert(sizeof(ActiveEnemy) == 3);
static_assert(EnemyRuntime::active_enemy_capacity == 60);
static_assert(EnemyRuntime::enemy_slot_capacity == 16);
static_assert(SpatialManager::actor_count == 17);
static_assert(EnemyRuntime::enemy_slot_capacity == SpatialManager::actor_count - 1);
static_assert(first_enemy_target_id + EnemyRuntime::enemy_slot_capacity - 1 == 25);
static_assert(uint8_t(EnemyType::GOBLIN) != uint8_t(EnemyType::CROSSBOW));

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
    BN_ASSERT(type == EnemyType::GOBLIN || type == EnemyType::CROSSBOW, "Unsupported enemy type");

    for(int slot_index = 0; slot_index < enemy_slot_capacity; ++slot_index)
    {
        EnemySlot& slot = _enemy_slots[slot_index];
        if(! slot.occupied())
        {
            const int target_id = first_enemy_target_id + slot_index;
            if(type == EnemyType::GOBLIN)
            {
                slot.construct_goblin(initial_home_position, target_id);
            }
            else
            {
                slot.construct_crossbow(initial_home_position, target_id);
            }

            return _register_enemy(uint8_t(slot_index), _allocate_actor_id());
        }
    }

    BN_ASSERT(false, "Enemy slots exhausted");
    return _active_enemies[0];
}

EnemyType EnemyRuntime::type(const ActiveEnemy& active_enemy) const
{
    return _slot(active_enemy).type();
}

Enemy& EnemyRuntime::enemy(const ActiveEnemy& active_enemy)
{
    return _slot(active_enemy).enemy();
}

const Enemy& EnemyRuntime::enemy(const ActiveEnemy& active_enemy) const
{
    return _slot(active_enemy).enemy();
}

Goblin& EnemyRuntime::goblin(const ActiveEnemy& active_enemy)
{
    return _slot(active_enemy).goblin();
}

const Goblin& EnemyRuntime::goblin(const ActiveEnemy& active_enemy) const
{
    return _slot(active_enemy).goblin();
}

CrossbowGoblin& EnemyRuntime::crossbow(const ActiveEnemy& active_enemy)
{
    return _slot(active_enemy).crossbow();
}

const CrossbowGoblin& EnemyRuntime::crossbow(const ActiveEnemy& active_enemy) const
{
    return _slot(active_enemy).crossbow();
}

EnemySlot& EnemyRuntime::_slot(const ActiveEnemy& active_enemy)
{
    BN_ASSERT(active_enemy.occupied, "Enemy roster entry is empty");
    BN_ASSERT(active_enemy.slot_index < enemy_slot_capacity, "Invalid enemy slot index");
    EnemySlot& result = _enemy_slots[active_enemy.slot_index];
    BN_ASSERT(result.occupied(), "Enemy slot is empty");
    return result;
}

const EnemySlot& EnemyRuntime::_slot(const ActiveEnemy& active_enemy) const
{
    BN_ASSERT(active_enemy.occupied, "Enemy roster entry is empty");
    BN_ASSERT(active_enemy.slot_index < enemy_slot_capacity, "Invalid enemy slot index");
    const EnemySlot& result = _enemy_slots[active_enemy.slot_index];
    BN_ASSERT(result.occupied(), "Enemy slot is empty");
    return result;
}

ActiveEnemy& EnemyRuntime::_register_enemy(uint8_t slot_index, SpatialActorId actor_id)
{
    BN_ASSERT(slot_index < enemy_slot_capacity);

    for(ActiveEnemy& enemy : _active_enemies)
    {
        if(! enemy.occupied)
        {
            enemy.occupied = true;
            enemy.slot_index = slot_index;
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

void EnemyRuntime::clear()
{
    for(EnemySlot& slot : _enemy_slots)
    {
        slot.destroy();
    }

    for(ActiveEnemy& enemy : _active_enemies)
    {
        enemy.occupied = false;
        enemy.actor_id = SpatialActorId::PLAYER;
        enemy.slot_index = 0;
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
