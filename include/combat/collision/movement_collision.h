#ifndef COLLISION_MOVEMENT_COLLISION_H
#define COLLISION_MOVEMENT_COLLISION_H

#include "combat/collision/collision_body.h"
#include "world/battlefield.h"

[[nodiscard]] bn::fixed_point resolve_movement(
        const bn::fixed_point& current_position,
        const bn::fixed_point& delta,
        const Pushbox& moving_pushbox,
        const WorldBoxList<3>& obstacles,
        const MovementBounds& bounds);

#endif
