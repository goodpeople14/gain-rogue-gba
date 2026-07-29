#include "character/training_dummy_manager.h"

#include "combat/melee/swordsman_attack.h"
#include "combat/collision/collision_math.h"

namespace
{
    constexpr int preferred_player_distance = 32;
    [[nodiscard]] bool nearer_than(const bn::fixed_point& first, const bn::fixed_point& second, int distance)
    {
        bn::fixed delta_x = first.x() - second.x();
        bn::fixed delta_y = first.y() - second.y();
        return (delta_x * delta_x) + (delta_y * delta_y) < distance * distance;
    }
}

TrainingDummyManager::TrainingDummyManager() :
    _dummy_1(1, 0, 0),
    _dummy_2(2, 0, 0),
    _dummy_3(3, 0, 0)
{
}

void TrainingDummyManager::enter()
{
    _dummy_1.reset();
    _dummy_2.reset();
    _dummy_3.reset();
    _spawn_timer = 0;
    _pending_slot = -1;
}

void TrainingDummyManager::update(const WorldBox& player_pushbox)
{
    for(int index = 0; index < max_dummies; ++index)
    {
        dummy(index).update();
    }

    if(active_count() == max_dummies)
    {
        _spawn_timer = 0;
        _pending_slot = -1;
        return;
    }

    if(_pending_slot >= 0)
    {
        try_complete_spawn(player_pushbox);
        return;
    }

    ++_spawn_timer;

    if(_spawn_timer >= spawn_interval_frames)
    {
        prepare_spawn_position(player_pushbox);
        try_complete_spawn(player_pushbox);
    }
}

void TrainingDummyManager::resolve_attack(SwordsmanAttack& attack)
{
    for(int index = 0; index < max_dummies; ++index)
    {
        TrainingDummy& target = dummy(index);

        if(target.active())
        {
            int damage = attack.try_hit(target.target_id(), target.position(), target.collision_body().hurtbox);
            target.receive_damage(damage);
        }
    }
}

int TrainingDummyManager::active_count() const
{
    int result = 0;

    for(int index = 0; index < max_dummies; ++index)
    {
        result += int(dummy(index).active());
    }

    return result;
}

WorldBoxList<TrainingDummyManager::max_dummies> TrainingDummyManager::active_pushboxes() const
{
    WorldBoxList<max_dummies> result;

    for(int index = 0; index < max_dummies; ++index)
    {
        const TrainingDummy& target = dummy(index);

        if(target.active())
        {
            result.boxes[result.count] = target.world_pushbox();
            ++result.count;
        }
    }

    return result;
}

TrainingDummy& TrainingDummyManager::dummy(int index)
{
    if(index == 0)
    {
        return _dummy_1;
    }

    return index == 1 ? _dummy_2 : _dummy_3;
}

const TrainingDummy& TrainingDummyManager::dummy(int index) const
{
    if(index == 0)
    {
        return _dummy_1;
    }

    return index == 1 ? _dummy_2 : _dummy_3;
}

int TrainingDummyManager::empty_slot() const
{
    for(int index = 0; index < max_dummies; ++index)
    {
        if(! dummy(index).active())
        {
            return index;
        }
    }

    return -1;
}

void TrainingDummyManager::prepare_spawn_position(const WorldBox& player_pushbox)
{
    _pending_slot = empty_slot();

    if(_pending_slot < 0)
    {
        return;
    }

    const Pushbox& spawn_pushbox = dummy(_pending_slot).collision_body().pushbox;
    bn::fixed_point fallback_position;
    bool has_fallback = false;

    for(int attempt = 0; attempt < spawn_attempts; ++attempt)
    {
        bn::fixed_point position(
                _random.get_int(spawn_bounds.min_x, spawn_bounds.max_x + 1),
                _random.get_int(spawn_bounds.min_y, spawn_bounds.max_y + 1));
        WorldBox candidate = world_box(position, spawn_pushbox.box);
        bool dummy_overlap = false;

        for(int index = 0; index < max_dummies; ++index)
        {
            const TrainingDummy& other = dummy(index);

            if(other.active() && overlaps_strictly(candidate, other.world_pushbox()))
            {
                dummy_overlap = true;
                break;
            }
        }

        if(! dummy_overlap)
        {
            if(! has_fallback)
            {
                fallback_position = position;
                has_fallback = true;
            }

            if(! overlaps_strictly(candidate, player_pushbox) &&
               ! nearer_than(position, player_pushbox.center, preferred_player_distance))
            {
                _pending_position = position;
                return;
            }
        }
    }

    if(has_fallback)
    {
        _pending_position = fallback_position;
    }
    else
    {
        _pending_slot = -1;
    }
}

bool TrainingDummyManager::spawn_position_occupied(const WorldBox& player_pushbox) const
{
    const Pushbox& spawn_pushbox = dummy(_pending_slot).collision_body().pushbox;
    WorldBox candidate = world_box(_pending_position, spawn_pushbox.box);

    if(overlaps_strictly(candidate, player_pushbox))
    {
        return true;
    }

    for(int index = 0; index < max_dummies; ++index)
    {
        const TrainingDummy& other = dummy(index);

        if(other.active() && overlaps_strictly(candidate, other.world_pushbox()))
        {
            return true;
        }
    }

    return false;
}

void TrainingDummyManager::try_complete_spawn(const WorldBox& player_pushbox)
{
    if(_pending_slot >= 0 && ! spawn_position_occupied(player_pushbox))
    {
        dummy(_pending_slot).respawn(_pending_position);
        _pending_slot = -1;
        _spawn_timer = 0;
    }
}
