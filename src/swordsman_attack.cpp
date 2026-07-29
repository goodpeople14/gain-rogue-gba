#include "combat/melee/swordsman_attack.h"

bool SwordsmanAttack::can_attack() const
{
    return _cooldown_remaining == 0;
}

bool SwordsmanAttack::try_attack(const AttackContext& context)
{
    if(! can_attack())
    {
        return false;
    }

    _hitbox.activate(context, hitbox_active_frames, attack_power);
    _cooldown_remaining = cooldown_frames;
    return true;
}

void SwordsmanAttack::update()
{
    if(_cooldown_remaining > 0)
    {
        --_cooldown_remaining;
    }

    _hitbox.update();
}

int SwordsmanAttack::try_hit(int target_id, const bn::fixed_point& target_center,
                             int target_width, int target_height)
{
    return _hitbox.try_hit(target_id, target_center, target_width, target_height);
}
