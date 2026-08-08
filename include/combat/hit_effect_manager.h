#ifndef COMBAT_HIT_EFFECT_MANAGER_H
#define COMBAT_HIT_EFFECT_MANAGER_H

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

class HitEffectManager
{
public:
    static constexpr int capacity = 8;
    static constexpr int frame_count = 4;
    static constexpr int ticks_per_frame = 2;

    void spawn(const bn::fixed_point& position);
    void update();
    void clear();

    [[nodiscard]] int active_count() const;

private:
    struct Slot
    {
        bn::optional<bn::sprite_ptr> sprite;
        int frame = 0;
        int ticks = 0;
    };

    bn::array<Slot, capacity> _slots;
};

#endif
