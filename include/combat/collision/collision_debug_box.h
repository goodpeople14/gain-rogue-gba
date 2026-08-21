#ifndef COMBAT_COLLISION_COLLISION_DEBUG_BOX_H
#define COMBAT_COLLISION_COLLISION_DEBUG_BOX_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

enum class CollisionDebugBoxType
{
    HURTBOX,
    HITBOX,
    PUSHBOX,
    COMMIT_BOX,
    RANGED_COMMIT_BOX,
    FLEE_BOX,
    STATIC_OBSTACLE
};

struct CollisionDebugBox
{
    WorldBox box;
    CollisionDebugBoxType type;
};

class CollisionDebugBoxList
{
public:
    // Conservative normal-scene maximum: player (2 + two melee hitboxes),
    // four melee goblins (3 each), four Crossbow body pairs, one focused
    // Crossbow's two ranged spacing boxes, every landing slot in the
    // four-arrow pool, and the Stage1 rock. Inactive actors are skipped.
    static constexpr int capacity = 31;

    bool add(const WorldBox& box, CollisionDebugBoxType type)
    {
        if(_count == capacity)
        {
            return false;
        }

        _boxes[_count] = { box, type };
        ++_count;
        return true;
    }

    [[nodiscard]] const bn::array<CollisionDebugBox, capacity>& boxes() const
    {
        return _boxes;
    }

    [[nodiscard]] int count() const
    {
        return _count;
    }

private:
    bn::array<CollisionDebugBox, capacity> _boxes = {};
    int _count = 0;
};

#endif
