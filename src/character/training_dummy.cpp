#include "character/training_dummy.h"

#include "bn_array.h"
#include "bn_color.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

namespace
{
    constexpr CollisionBody training_dummy_collision_body = {
        { { 0, 0, 10, 10 } },
        { { 0, 2, 10, 10 } }
    };

    constexpr int dummy_pixel(int x, int y)
    {
        if(y >= 2 && y <= 5 && x >= 5 && x <= 10)
        {
            return 1;
        }

        if(y >= 6 && y <= 11 && x >= 3 && x <= 12)
        {
            return 2;
        }

        if(y >= 12 && y <= 14 && x >= 7 && x <= 8)
        {
            return 3;
        }

        return 0;
    }

    constexpr bn::array<bn::tile, 4> make_dummy_tiles()
    {
        bn::array<bn::tile, 4> result = {};

        for(int y = 0; y < 16; ++y)
        {
            for(int x = 0; x < 16; ++x)
            {
                int color_index = dummy_pixel(x, y);

                if(color_index)
                {
                    int tile_index = ((y / 8) * 2) + (x / 8);
                    result[tile_index].data[y % 8] |= unsigned(color_index) << ((x % 8) * 4);
                }
            }
        }

        return result;
    }

    constexpr bn::array<bn::tile, 4> dummy_tiles = make_dummy_tiles();

    constexpr bn::array<bn::color, 16> dummy_colors = {
        bn::color(0, 0, 0), bn::color(24, 17, 8), bn::color(17, 10, 4), bn::color(12, 7, 3),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::sprite_item dummy_item(
            bn::sprite_shape_size(16, 16), dummy_tiles, dummy_colors, bn::bpp_mode::BPP_4, 1);
}

TrainingDummy::TrainingDummy(int target_id, bn::fixed x, bn::fixed y) :
    _sprite(dummy_item.create_sprite(x, y)),
    _target_id(target_id)
{
    _sprite.set_visible(false);
}

void TrainingDummy::reset()
{
    _current_hp = 0;
    _flash_frames = 0;
    _active = false;
    _sprite.set_visible(false);
}

void TrainingDummy::respawn(const bn::fixed_point& position)
{
    _sprite.set_position(position);
    _current_hp = max_hp;
    _flash_frames = 0;
    _active = true;
    _sprite.set_visible(true);
}

void TrainingDummy::update()
{
    if(! _active)
    {
        return;
    }

    if(_flash_frames > 0)
    {
        _sprite.set_visible((_flash_frames % 2) == 0);
        --_flash_frames;

        if(! _flash_frames)
        {
            _sprite.set_visible(true);
        }
    }
}

void TrainingDummy::receive_damage(int damage)
{
    if(! _active || damage <= 0)
    {
        return;
    }

    _current_hp -= damage;

    if(_current_hp <= 0)
    {
        _current_hp = 0;
        _active = false;
        _sprite.set_visible(false);
    }
    else
    {
        _flash_frames = 5;
    }
}

bool TrainingDummy::active() const
{
    return _active;
}

int TrainingDummy::target_id() const
{
    return _target_id;
}

int TrainingDummy::current_hp() const
{
    return _current_hp;
}

bn::fixed_point TrainingDummy::position() const
{
    return _sprite.position();
}

const CollisionBody& TrainingDummy::collision_body() const
{
    return training_dummy_collision_body;
}

WorldBox TrainingDummy::world_hurtbox() const
{
    return world_box(position(), training_dummy_collision_body.hurtbox.box);
}

WorldBox TrainingDummy::world_pushbox() const
{
    return world_box(position(), training_dummy_collision_body.pushbox.box);
}
