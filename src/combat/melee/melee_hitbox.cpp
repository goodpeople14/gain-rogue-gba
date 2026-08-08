#include "combat/melee/melee_hitbox.h"

#include "combat/collision/collision_math.h"
#include "combat/melee/swordsman_attack_data.h"

namespace
{
    [[nodiscard]] constexpr bool hit_registry_tests()
    {
        AttackHitRegistry registry;

        if(! registry.add(1) || registry.add(1) || ! registry.add(2))
        {
            return false;
        }

        registry.reset();
        return registry.add(1);
    }

    static_assert(hit_registry_tests());

    static_assert(touches_or_intersects({ { 0, 0 }, 8, 8 }, { { 9, 0 }, 10, 10 }));
    static_assert(! overlaps_strictly({ { 0, 0 }, 8, 8 }, { { 9, 0 }, 10, 10 }));
}

void MeleeHitbox::activate(const AttackContext& context, int active_frames, int attack_power)
{
    _context = context;
    _game_frame = 0;
    _total_frames = active_frames;
    _attack_power = attack_power;
    _hit_registry.reset();
}

void MeleeHitbox::update()
{
    if(_game_frame == 0)
    {
        if(_total_frames > 0)
        {
            _game_frame = 1;
        }

        return;
    }

    if(_game_frame < _total_frames)
    {
        ++_game_frame;
    }
    else
    {
        _game_frame = 0;
        _total_frames = 0;
    }
}

bool MeleeHitbox::active() const
{
    return _game_frame > 0 && _game_frame <= _total_frames;
}

WorldBoxList<max_hitboxes_per_frame> MeleeHitbox::active_hitboxes() const
{
    WorldBoxList<max_hitboxes_per_frame> result;
    if(! active())
    {
        return result;
    }

    const AttackFrameData& frame_data = swordsman_attack_frame_data(_context.direction, _game_frame);
    for(int index = 0; index < frame_data.hitbox_count; ++index)
    {
        result.boxes[index] = world_box(_context.position, frame_data.hitboxes[index].box);
    }
    result.count = frame_data.hitbox_count;
    return result;
}

int MeleeHitbox::try_hit(int target_id, const bn::fixed_point& target_position,
                         const Hurtbox& target_hurtbox)
{
    if(! active() || _hit_registry.contains(target_id))
    {
        return 0;
    }

    WorldBox hurtbox = world_box(target_position, target_hurtbox.box);
    WorldBoxList<max_hitboxes_per_frame> hitboxes = active_hitboxes();

    for(int index = 0; index < hitboxes.count; ++index)
    {
        if(touches_or_intersects(hitboxes.boxes[index], hurtbox))
        {
            _hit_registry.add(target_id);
            return _attack_power;
        }
    }

    return 0;
}
