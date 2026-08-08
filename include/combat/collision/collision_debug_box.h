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
    static constexpr int capacity = 16;

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
