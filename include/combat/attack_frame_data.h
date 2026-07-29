#ifndef COMBAT_ATTACK_FRAME_DATA_H
#define COMBAT_ATTACK_FRAME_DATA_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

constexpr int max_hitboxes_per_frame = 2;

struct AttackFrameData
{
    bn::array<Hitbox, max_hitboxes_per_frame> hitboxes = {};
    int hitbox_count = 0;
    int image_frame = 1;
};

#endif
