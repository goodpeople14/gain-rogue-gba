#ifndef COMBAT_MELEE_MELEE_HITBOX_H
#define COMBAT_MELEE_MELEE_HITBOX_H

#include "bn_array.h"
#include "bn_sprite_ptr.h"

#include "combat/attack_context.h"
#include "combat/attack_hit_registry.h"
#include "combat/collision/collision_box.h"

class MeleeHitbox
{
public:
    static constexpr int effect_sprite_count = 2;

    MeleeHitbox();

    void activate(const AttackContext& context, int active_frames, int attack_power);
    void update();

    [[nodiscard]] bool active() const;
    [[nodiscard]] int try_hit(int target_id, const bn::fixed_point& target_position,
                              const Hurtbox& target_hurtbox);

private:
    void _refresh_effects();

    bn::array<bn::sprite_ptr, effect_sprite_count> _effect_sprites;
    AttackHitRegistry _hit_registry;
    AttackContext _context = {};
    int _game_frame = 0;
    int _total_frames = 0;
    int _attack_power = 0;
};

#endif
