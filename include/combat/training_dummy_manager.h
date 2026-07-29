#ifndef COMBAT_TRAINING_DUMMY_MANAGER_H
#define COMBAT_TRAINING_DUMMY_MANAGER_H

#include "bn_fixed_point.h"
#include "bn_random.h"

#include "battlefield.h"
#include "combat/training_dummy.h"

class SwordsmanAttack;

class TrainingDummyManager
{
public:
    static constexpr int max_dummies = 3;
    static constexpr int spawn_interval_frames = 60;
    static constexpr int spawn_attempts = 16;

    TrainingDummyManager();

    void enter();
    void update(const bn::fixed_point& player_position);
    void resolve_attack(SwordsmanAttack& attack);

    [[nodiscard]] int active_count() const;

private:
    [[nodiscard]] TrainingDummy& dummy(int index);
    [[nodiscard]] const TrainingDummy& dummy(int index) const;
    [[nodiscard]] int empty_slot() const;
    [[nodiscard]] bool valid_position(const bn::fixed_point& position,
                                      const bn::fixed_point& player_position) const;
    bool try_spawn(const bn::fixed_point& player_position);

    static constexpr MovementBounds spawn_bounds =
            Battlefield::movement_bounds(TrainingDummy::size, TrainingDummy::size);

    TrainingDummy _dummy_1;
    TrainingDummy _dummy_2;
    TrainingDummy _dummy_3;
    bn::random _random;
    int _spawn_timer = 0;
};

#endif
