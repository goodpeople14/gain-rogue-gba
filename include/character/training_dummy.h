#ifndef COMBAT_TRAINING_DUMMY_H
#define COMBAT_TRAINING_DUMMY_H

#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"

#include "combat/collision/collision_body.h"

class TrainingDummy
{
public:
    static constexpr int size = 16;
    static constexpr int max_hp = 3;
    TrainingDummy(int target_id, bn::fixed x, bn::fixed y);

    void reset();
    void respawn(const bn::fixed_point& position);
    void update();
    void receive_damage(int damage);

    [[nodiscard]] int target_id() const;
    [[nodiscard]] bool active() const;
    [[nodiscard]] int current_hp() const;
    [[nodiscard]] bn::fixed_point position() const;
    [[nodiscard]] const CollisionBody& collision_body() const;
    [[nodiscard]] WorldBox world_hurtbox() const;
    [[nodiscard]] WorldBox world_pushbox() const;

private:
    bn::sprite_ptr _sprite;
    int _target_id;
    int _current_hp = 0;
    int _flash_frames = 0;
    bool _active = false;
};

#endif
