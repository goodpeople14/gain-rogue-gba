#ifndef COMBAT_COLLISION_COLLISION_DEBUG_BOX_H
#define COMBAT_COLLISION_COLLISION_DEBUG_BOX_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

enum class CollisionDebugBoxType
{
    HURTBOX,
    HITBOX,
    PUSHBOX,
    COMMIT_BOX
};

struct CollisionDebugBox
{
    WorldBox box;
    CollisionDebugBoxType type;
};

class CollisionDebugBoxList
{
public:
    // Player (2 + two melee hitboxes), four melee goblins (3 each), one
    // crossbow goblin (two bodies plus six directional Commit cells), and
    // every landing slot in its four-arrow pool.
    static constexpr int capacity = 28;

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
