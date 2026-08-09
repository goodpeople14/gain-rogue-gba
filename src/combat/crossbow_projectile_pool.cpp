#include "combat/crossbow_projectile_pool.h"

#include "bn_sprite_items_crossbow_arrow.h"

#include "combat/collision/collision_math.h"
#include "combat/hit_effect_manager.h"

namespace
{
    constexpr int flight_frames = 36;
    constexpr int peak_height = 8;
    constexpr LocalBox landing_collision_box = { 0, 0, 8, 8 };

    [[nodiscard]] bn::fixed interpolate(bn::fixed start, bn::fixed target, int ticks)
    {
        return start + ((target - start) * ticks) / flight_frames;
    }

    [[nodiscard]] constexpr int height_offset(int ticks)
    {
        int rising_ticks = ticks <= flight_frames / 2 ? ticks : flight_frames - ticks;
        return (rising_ticks * peak_height) / (flight_frames / 2);
    }

    [[nodiscard]] WorldBox landing_hitbox(const bn::fixed_point& target)
    {
        return world_box(target, landing_collision_box);
    }

    static_assert(height_offset(0) == 0);
    static_assert(height_offset(flight_frames / 2) == peak_height);
    static_assert(height_offset(flight_frames) == 0);
}

void CrossbowProjectilePool::spawn(const bn::fixed_point& start, const bn::fixed_point& target)
{
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            slot.sprite = bn::sprite_items::crossbow_arrow.create_sprite(start);
            slot.sprite->set_z_order(-1);
            slot.start = start;
            slot.target = target;
            slot.ticks = 0;
            slot.landing = false;
            slot.hit_resolved = false;
            return;
        }
    }
}

void CrossbowProjectilePool::update()
{
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            continue;
        }

        if(slot.landing)
        {
            slot.sprite.reset();
            continue;
        }

        ++slot.ticks;
        if(slot.ticks >= flight_frames)
        {
            slot.sprite->set_position(slot.target);
            slot.landing = true;
            continue;
        }

        bn::fixed_point position(interpolate(slot.start.x(), slot.target.x(), slot.ticks),
                                 interpolate(slot.start.y(), slot.target.y(), slot.ticks) - height_offset(slot.ticks));
        slot.sprite->set_position(position);
    }
}

void CrossbowProjectilePool::resolve_player_hit(const bn::fixed_point& player_position,
                                                 const Hurtbox& player_hurtbox, HitEffectManager& hit_effects)
{
    WorldBox hurtbox = world_box(player_position, player_hurtbox.box);
    for(Slot& slot : _slots)
    {
        if(slot.sprite && slot.landing && ! slot.hit_resolved)
        {
            slot.hit_resolved = true;
            if(touches_or_intersects(landing_hitbox(slot.target), hurtbox))
            {
                hit_effects.spawn(hurtbox.center);
            }
        }
    }
}

void CrossbowProjectilePool::append_collision_debug_boxes(CollisionDebugBoxList& boxes) const
{
    for(const Slot& slot : _slots)
    {
        if(slot.sprite && slot.landing)
        {
            boxes.add(landing_hitbox(slot.target), CollisionDebugBoxType::HITBOX);
        }
    }
}

void CrossbowProjectilePool::clear()
{
    for(Slot& slot : _slots)
    {
        slot.sprite.reset();
        slot.ticks = 0;
        slot.landing = false;
        slot.hit_resolved = false;
    }
}

int CrossbowProjectilePool::active_count() const
{
    int result = 0;
    for(const Slot& slot : _slots)
    {
        result += int(bool(slot.sprite));
    }
    return result;
}
