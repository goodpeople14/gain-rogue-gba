#ifndef COMBAT_COLLISION_COLLISION_DEBUG_BOX_H
#define COMBAT_COLLISION_COLLISION_DEBUG_BOX_H

#include "bn_array.h"

#include "combat/collision/collision_box.h"

enum class CollisionDebugBoxType
{
    HURTBOX,
    HITBOX,
    PUSHBOX,
    STATIC_OBSTACLE
};

enum class CollisionDebugRadiusType
{
    DISCOVERY,
    DISENGAGE,
    MELEE_COMMIT,
    RANGED_COMMIT,
    FLEE
};

struct CollisionDebugBox
{
    WorldBox box;
    CollisionDebugBoxType type;
};

class CollisionDebugBoxList
{
public:
    // Hero (hurtbox, pushbox and two active hitboxes), one selected Crossbow
    // body pair, and its four projectile landing hitboxes.
    static constexpr int capacity = 10;

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

struct CollisionDebugRadius
{
    bn::fixed_point center;
    int radius;
    CollisionDebugRadiusType type;
};

class CollisionDebugRadiusList
{
public:
    // Crossbow is the worst case: discovery, disengage, ranged commit and flee.
    static constexpr int capacity = 4;

    bool add(const bn::fixed_point& center, int radius, CollisionDebugRadiusType type)
    {
        if(_count == capacity)
        {
            return false;
        }

        _radii[_count] = { center, radius, type };
        ++_count;
        return true;
    }

    [[nodiscard]] const bn::array<CollisionDebugRadius, capacity>& radii() const
    {
        return _radii;
    }

    [[nodiscard]] int count() const
    {
        return _count;
    }

private:
    bn::array<CollisionDebugRadius, capacity> _radii = {};
    int _count = 0;
};

#endif
