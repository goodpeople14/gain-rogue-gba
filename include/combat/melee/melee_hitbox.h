#ifndef COMBAT_MELEE_MELEE_HITBOX_H
#define COMBAT_MELEE_MELEE_HITBOX_H

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"

#include "combat/attack_context.h"

struct AttackArea
{
    static constexpr int size = 16;

    bn::fixed_point center;

    [[nodiscard]] bool intersects(const bn::fixed_point& target_center,
                                  int target_width, int target_height) const;
};

class MeleeHitbox
{
public:
    static constexpr int area_count = 3;

    MeleeHitbox();

    void activate(const AttackContext& context, int active_frames, int attack_power);
    void update();

    [[nodiscard]] bool active() const;
    [[nodiscard]] int try_hit(int target_id, const bn::fixed_point& target_center,
                              int target_width, int target_height);

private:
    static constexpr int max_hit_targets = 8;

    bn::array<AttackArea, area_count> _areas;
    bn::array<bn::sprite_ptr, area_count> _effect_sprites;
    bn::array<int, max_hit_targets> _hit_target_ids;
    int _hit_target_count = 0;
    int _remaining_frames = 0;
    int _attack_power = 0;
    bool _newly_activated = false;
};

#endif
