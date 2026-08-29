#ifndef ENEMY_ENEMY_RUNTIME_H
#define ENEMY_ENEMY_RUNTIME_H

#include "bn_array.h"
#include "bn_fixed.h"

#include "enemy/enemy_slot.h"
#include "enemy/enemy_type.h"
#include "world/spatial_manager.h"

struct ActiveEnemy
{
    bool occupied = false;
    SpatialActorId actor_id = SpatialActorId::PLAYER;
    uint8_t slot_index = 0;
};

class EnemyRuntime
{
public:
    static constexpr int enemy_slot_capacity = 16;
    static constexpr int active_enemy_capacity = 60;

    ActiveEnemy& active_enemy(int index);
    const ActiveEnemy& active_enemy(int index) const;
    ActiveEnemy& allocate_enemy(EnemyType type);
    [[nodiscard]] EnemyType type(const ActiveEnemy& active_enemy) const;
    [[nodiscard]] Enemy& enemy(const ActiveEnemy& active_enemy);
    [[nodiscard]] const Enemy& enemy(const ActiveEnemy& active_enemy) const;
    [[nodiscard]] Goblin& goblin(const ActiveEnemy& active_enemy);
    [[nodiscard]] const Goblin& goblin(const ActiveEnemy& active_enemy) const;
    [[nodiscard]] CrossbowGoblin& crossbow(const ActiveEnemy& active_enemy);
    [[nodiscard]] const CrossbowGoblin& crossbow(const ActiveEnemy& active_enemy) const;
    void clear();
    [[nodiscard]] int roster_count() const;

private:
    [[nodiscard]] EnemySlot& _slot(const ActiveEnemy& active_enemy);
    [[nodiscard]] const EnemySlot& _slot(const ActiveEnemy& active_enemy) const;
    ActiveEnemy& _register_enemy(uint8_t slot_index, SpatialActorId actor_id);
    [[nodiscard]] SpatialActorId _allocate_actor_id() const;
    bn::array<EnemySlot, enemy_slot_capacity> _enemy_slots;
    bn::array<ActiveEnemy, active_enemy_capacity> _active_enemies = {};
};

#endif
