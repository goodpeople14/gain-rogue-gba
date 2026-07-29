#ifndef COMBAT_TRAINING_DUMMY_MANAGER_H
#define COMBAT_TRAINING_DUMMY_MANAGER_H

#include "bn_fixed_point.h"
#include "bn_random.h"

#include "character/training_dummy.h"
#include "combat/collision/collision_body.h"
#include "world/battlefield.h"

class SwordsmanAttack;

class TrainingDummyManager
{
public:
    static constexpr int max_dummies = 3;
    static constexpr int spawn_interval_frames = 60;
    static constexpr int spawn_attempts = 16;

    TrainingDummyManager();

    void enter();
    void update(const WorldBox& player_pushbox);
    void resolve_attack(SwordsmanAttack& attack);

    [[nodiscard]] int active_count() const;
    [[nodiscard]] WorldBoxList<max_dummies> active_pushboxes() const;

private:
    [[nodiscard]] TrainingDummy& dummy(int index);
    [[nodiscard]] const TrainingDummy& dummy(int index) const;
    [[nodiscard]] int empty_slot() const;
    void prepare_spawn_position(const WorldBox& player_pushbox);
    [[nodiscard]] bool spawn_position_occupied(const WorldBox& player_pushbox) const;
    void try_complete_spawn(const WorldBox& player_pushbox);

    static constexpr MovementBounds spawn_bounds =
            Battlefield::movement_bounds(TrainingDummy::size, TrainingDummy::size);

    TrainingDummy _dummy_1;
    TrainingDummy _dummy_2;
    TrainingDummy _dummy_3;
    bn::random _random;
    int _spawn_timer = 0;
    int _pending_slot = -1;
    bn::fixed_point _pending_position;
};

#endif
