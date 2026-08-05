#include "combat/hit_effect_manager.h"

#include "bn_sprite_items_hit_effect.h"

void HitEffectManager::spawn(const bn::fixed_point& position)
{
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            slot.sprite = bn::sprite_items::hit_effect.create_sprite(position, 0);
            slot.frame = 0;
            slot.ticks = 0;
            return;
        }
    }
}

void HitEffectManager::update()
{
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            continue;
        }

        ++slot.ticks;
        if(slot.ticks < ticks_per_frame)
        {
            continue;
        }

        slot.ticks = 0;
        ++slot.frame;
        if(slot.frame == frame_count)
        {
            slot.sprite.reset();
        }
        else
        {
            slot.sprite->set_item(bn::sprite_items::hit_effect, slot.frame);
        }
    }
}

void HitEffectManager::clear()
{
    for(Slot& slot : _slots)
    {
        slot.sprite.reset();
        slot.frame = 0;
        slot.ticks = 0;
    }
}

int HitEffectManager::active_count() const
{
    int result = 0;
    for(const Slot& slot : _slots)
    {
        result += int(bool(slot.sprite));
    }
    return result;
}
