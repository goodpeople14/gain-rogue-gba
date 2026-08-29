#ifndef ENEMY_ENEMY_SLOT_H
#define ENEMY_ENEMY_SLOT_H

#include "bn_fixed_point.h"

#include "character/crossbow_goblin.h"
#include "character/goblin.h"
#include "enemy/enemy_type.h"

class EnemySlot
{
public:
    EnemySlot() = default;
    EnemySlot(const EnemySlot&) = delete;
    EnemySlot& operator=(const EnemySlot&) = delete;
    EnemySlot(EnemySlot&&) = delete;
    EnemySlot& operator=(EnemySlot&&) = delete;
    ~EnemySlot();

    [[nodiscard]] bool occupied() const;
    [[nodiscard]] EnemyType type() const;

    Goblin& construct_goblin(const bn::fixed_point& position, int target_id);
    CrossbowGoblin& construct_crossbow(const bn::fixed_point& position, int target_id);
    void destroy();

    [[nodiscard]] Enemy& enemy();
    [[nodiscard]] const Enemy& enemy() const;
    [[nodiscard]] Goblin& goblin();
    [[nodiscard]] const Goblin& goblin() const;
    [[nodiscard]] CrossbowGoblin& crossbow();
    [[nodiscard]] const CrossbowGoblin& crossbow() const;

private:
    union Storage
    {
        char empty;
        Goblin goblin;
        CrossbowGoblin crossbow;

        constexpr Storage() :
            empty(0)
        {
        }

        ~Storage()
        {
        }
    };

    Storage _storage;
    EnemyType _type = EnemyType::NONE;
};

#endif
