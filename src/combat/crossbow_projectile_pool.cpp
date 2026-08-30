#include "combat/crossbow_projectile_pool.h"

#include "bn_sprite_items_crossbow_arrow.h"

#include "combat/collision/collision_math.h"
#include "combat/hit_effect_manager.h"

#if defined(GAIN_PERF_DEBUG_LOGS)
    #include "debug/perf_stats.h"
#endif

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

    [[nodiscard]] constexpr bool matches_debug_source(int slot_source_actor_id, int selected_actor_id)
    {
        return slot_source_actor_id == selected_actor_id;
    }

    static_assert(height_offset(0) == 0);
    static_assert(height_offset(flight_frames / 2) == peak_height);
    static_assert(height_offset(flight_frames) == 0);
    static_assert(matches_debug_source(14, 14));
    static_assert(! matches_debug_source(14, 15));
}

void CrossbowProjectilePool::spawn(
        int source_actor_id, const bn::fixed_point& start, const bn::fixed_point& target)
{
#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().projectile_spawn_attempts;
#endif
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            slot.sprite = bn::sprite_items::crossbow_arrow.create_sprite(start);
            slot.sprite->set_z_order(-1);
            slot.start = start;
            slot.target = target;
            slot.source_actor_id = source_actor_id;
            slot.ticks = 0;
            slot.landing = false;
            slot.hit_resolved = false;
#if defined(GAIN_PERF_DEBUG_LOGS)
            ++perf_stats().projectile_spawn_success;
#endif
            return;
        }
    }

#if defined(GAIN_PERF_DEBUG_LOGS)
    ++perf_stats().projectile_spawn_dropped_pool_full;
#endif
}

void CrossbowProjectilePool::update()
{
#if defined(GAIN_PERF_DEBUG_LOGS)
    int active_count = 0;
#endif
    for(Slot& slot : _slots)
    {
        if(! slot.sprite)
        {
            continue;
        }

#if defined(GAIN_PERF_DEBUG_LOGS)
        ++active_count;
#endif

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

#if defined(GAIN_PERF_DEBUG_LOGS)
    if(active_count > perf_stats().active_projectile_max)
    {
        perf_stats().active_projectile_max = active_count;
    }
#endif
}

int CrossbowProjectilePool::resolve_player_hit(const bn::fixed_point& player_position,
                                                const Hurtbox& player_hurtbox, HitEffectManager& hit_effects)
{
    int result = 0;
    WorldBox hurtbox = world_box(player_position, player_hurtbox.box);
    for(Slot& slot : _slots)
    {
        if(slot.sprite && slot.landing && ! slot.hit_resolved)
        {
            slot.hit_resolved = true;
            if(touches_or_intersects(landing_hitbox(slot.target), hurtbox))
            {
                hit_effects.spawn(hurtbox.center);
                ++result;
            }
        }
    }

    return result;
}

void CrossbowProjectilePool::append_collision_debug_boxes(
        int source_actor_id, CollisionDebugBoxList& boxes) const
{
    for(const Slot& slot : _slots)
    {
        if(slot.sprite && matches_debug_source(slot.source_actor_id, source_actor_id) && slot.landing)
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
        slot.source_actor_id = 0;
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
