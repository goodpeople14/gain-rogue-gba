#ifndef COMBAT_ATTACK_HIT_REGISTRY_H
#define COMBAT_ATTACK_HIT_REGISTRY_H

#include "bn_array.h"

class AttackHitRegistry
{
public:
    static constexpr int capacity = 8;

    constexpr void reset()
    {
        _count = 0;
    }

    [[nodiscard]] constexpr bool contains(int target_id) const
    {
        for(int index = 0; index < _count; ++index)
        {
            if(_target_ids[index] == target_id)
            {
                return true;
            }
        }

        return false;
    }

    constexpr bool add(int target_id)
    {
        if(contains(target_id) || _count == capacity)
        {
            return false;
        }

        _target_ids[_count] = target_id;
        ++_count;
        return true;
    }

private:
    bn::array<int, capacity> _target_ids = {};
    int _count = 0;
};

#endif
