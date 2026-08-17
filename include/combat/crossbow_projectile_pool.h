#ifndef COMBAT_CROSSBOW_PROJECTILE_POOL_H
#define COMBAT_CROSSBOW_PROJECTILE_POOL_H

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

#include "combat/collision/collision_debug_box.h"

class HitEffectManager;

// This deliberately serves only the crossbow goblin prototype.  It has no terrain
// collision: an active arrow travels to its launch-time target independently of its owner.
class CrossbowProjectilePool
{
public:
    static constexpr int capacity = 4;

    void spawn(const bn::fixed_point& start, const bn::fixed_point& target);
    void update();
    [[nodiscard]] int resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                                         HitEffectManager& hit_effects);
    void append_collision_debug_boxes(CollisionDebugBoxList& boxes) const;
    void clear();

    [[nodiscard]] int active_count() const;

private:
    struct Slot
    {
        bn::optional<bn::sprite_ptr> sprite;
        bn::fixed_point start;
        bn::fixed_point target;
        int ticks = 0;
        bool landing = false;
        bool hit_resolved = false;
    };

    bn::array<Slot, capacity> _slots;
};

#endif
