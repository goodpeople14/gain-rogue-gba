#ifndef COMBAT_MELEE_MELEE_HITBOX_H
#define COMBAT_MELEE_MELEE_HITBOX_H

#include "combat/attack_context.h"
#include "combat/attack_hit_registry.h"
#include "combat/collision/collision_box.h"

class MeleeHitbox
{
public:
    void activate(const AttackContext& context, int active_frames, int attack_power);
    void update();

    [[nodiscard]] bool active() const;
    [[nodiscard]] int try_hit(int target_id, const bn::fixed_point& target_position,
                              const Hurtbox& target_hurtbox);

private:
    AttackHitRegistry _hit_registry;
    AttackContext _context = {};
    int _game_frame = 0;
    int _total_frames = 0;
    int _attack_power = 0;
};

#endif
