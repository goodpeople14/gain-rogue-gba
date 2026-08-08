#ifndef COMBAT_MELEE_SWORDSMAN_SLASH_EFFECT_H
#define COMBAT_MELEE_SWORDSMAN_SLASH_EFFECT_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

#include "combat/attack_context.h"

class SwordsmanSlashEffect
{
public:
    static constexpr int frame_count = 5;
    static constexpr int ticks_per_frame = 2;

    void play(const AttackContext& context);
    void update(const bn::fixed_point& position);

    [[nodiscard]] bool active() const;

private:
    bn::optional<bn::sprite_ptr> _sprite;
    Direction _direction = Direction::DOWN;
    int _frame = 0;
    int _ticks_in_frame = 0;
};

#endif
