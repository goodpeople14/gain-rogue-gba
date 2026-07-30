#ifndef COMBAT_MELEE_SWORDSMAN_ATTACK_VISUAL_H
#define COMBAT_MELEE_SWORDSMAN_ATTACK_VISUAL_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

#include "combat/attack_context.h"

class SwordsmanAttackVisual
{
public:
    static constexpr int frame_count = 5;
    static constexpr int ticks_per_frame = 2;

    void play(const AttackContext& context);
    void update();

    [[nodiscard]] bool active() const;

private:
    bn::optional<bn::sprite_ptr> _sword_sprite;
    bn::optional<bn::sprite_ptr> _slash_sprite;
    Direction _direction = Direction::DOWN;
    int _frame = 0;
    int _ticks_in_frame = 0;
};

#endif
