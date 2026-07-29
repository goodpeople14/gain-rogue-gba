#include "combat/training_dummy_manager.h"

#include "combat/melee/swordsman_attack.h"

namespace
{
    constexpr int preferred_player_distance = 32;
    constexpr int minimum_dummy_spacing = 16;

    [[nodiscard]] bn::fixed absolute(bn::fixed value)
    {
        return value < 0 ? -value : value;
    }

    [[nodiscard]] bool overlaps(const bn::fixed_point& first, const bn::fixed_point& second, int size)
    {
        return absolute(first.x() - second.x()) < size && absolute(first.y() - second.y()) < size;
    }

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
}

void TrainingDummyManager::update(const bn::fixed_point& player_position)
{
    for(int index = 0; index < max_dummies; ++index)
    {
        dummy(index).update();
    }

    if(active_count() == max_dummies)
    {
        _spawn_timer = 0;
        return;
    }

    ++_spawn_timer;

    if(_spawn_timer >= spawn_interval_frames && try_spawn(player_position))
    {
        _spawn_timer = 0;
    }
}

void TrainingDummyManager::resolve_attack(SwordsmanAttack& attack)
{
    for(int index = 0; index < max_dummies; ++index)
    {
        TrainingDummy& target = dummy(index);

        if(target.active())
        {
            int damage = attack.try_hit(
                    target.target_id(), target.position(), TrainingDummy::size, TrainingDummy::size);
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

bool TrainingDummyManager::valid_position(const bn::fixed_point& position,
                                           const bn::fixed_point& player_position) const
{
    if(overlaps(position, player_position, TrainingDummy::size) ||
       nearer_than(position, player_position, preferred_player_distance))
    {
        return false;
    }

    for(int index = 0; index < max_dummies; ++index)
    {
        const TrainingDummy& other = dummy(index);

        if(other.active() && (overlaps(position, other.position(), TrainingDummy::size) ||
                             nearer_than(position, other.position(), minimum_dummy_spacing)))
        {
            return false;
        }
    }

    return true;
}

bool TrainingDummyManager::try_spawn(const bn::fixed_point& player_position)
{
    int slot = empty_slot();

    if(slot < 0)
    {
        return false;
    }

    for(int attempt = 0; attempt < spawn_attempts; ++attempt)
    {
        int x = _random.get_int(spawn_bounds.min_x, spawn_bounds.max_x + 1);
        int y = _random.get_int(spawn_bounds.min_y, spawn_bounds.max_y + 1);
        bn::fixed_point position(x, y);

        if(valid_position(position, player_position))
        {
            dummy(slot).respawn(position);
            return true;
        }
    }

    return false;
}
