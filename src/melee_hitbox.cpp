#include "combat/melee/melee_hitbox.h"

#include "bn_color.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

namespace
{
    struct AttackVectors
    {
        bn::fixed_point forward;
        bn::fixed_point side;
    };

    constexpr bn::array<bn::tile, 4> make_effect_tiles()
    {
        bn::array<bn::tile, 4> result = {};

        for(int y = 0; y < 16; ++y)
        {
            for(int x = 0; x < 16; ++x)
            {
                if(x == y || x + y == 15)
                {
                    int tile_index = ((y / 8) * 2) + (x / 8);
                    result[tile_index].data[y % 8] |= 1U << ((x % 8) * 4);
                }
            }
        }

        return result;
    }

    constexpr bn::array<bn::tile, 4> effect_tiles = make_effect_tiles();

    constexpr bn::array<bn::color, 16> effect_colors = {
        bn::color(0, 0, 0), bn::color(31, 27, 6), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::sprite_item effect_item(
            bn::sprite_shape_size(16, 16), effect_tiles, effect_colors, bn::bpp_mode::BPP_4, 1);

    [[nodiscard]] AttackVectors attack_vectors(Direction direction)
    {
        switch(direction)
        {
        case Direction::DOWN: return { bn::fixed_point(0, 16), bn::fixed_point(-16, 0) };
        case Direction::DOWN_LEFT: return { bn::fixed_point(-12, 12), bn::fixed_point(-12, -12) };
        case Direction::LEFT: return { bn::fixed_point(-16, 0), bn::fixed_point(0, -16) };
        case Direction::UP_LEFT: return { bn::fixed_point(-12, -12), bn::fixed_point(12, -12) };
        case Direction::UP: return { bn::fixed_point(0, -16), bn::fixed_point(16, 0) };
        case Direction::UP_RIGHT: return { bn::fixed_point(12, -12), bn::fixed_point(12, 12) };
        case Direction::RIGHT: return { bn::fixed_point(16, 0), bn::fixed_point(0, 16) };
        case Direction::DOWN_RIGHT: return { bn::fixed_point(12, 12), bn::fixed_point(-12, 12) };
        default: return { bn::fixed_point(0, 16), bn::fixed_point(-16, 0) };
        }
    }
}

bool AttackArea::intersects(const bn::fixed_point& target_center, int target_width, int target_height) const
{
    bn::fixed half_size = size / 2;
    bn::fixed target_half_width = target_width / 2;
    bn::fixed target_half_height = target_height / 2;

    return center.x() - half_size < target_center.x() + target_half_width &&
           center.x() + half_size > target_center.x() - target_half_width &&
           center.y() - half_size < target_center.y() + target_half_height &&
           center.y() + half_size > target_center.y() - target_half_height;
}

MeleeHitbox::MeleeHitbox() :
    _effect_sprites {
        effect_item.create_sprite(0, 0),
        effect_item.create_sprite(0, 0),
        effect_item.create_sprite(0, 0)
    }
{
    for(bn::sprite_ptr& sprite : _effect_sprites)
    {
        sprite.set_visible(false);
    }
}

void MeleeHitbox::activate(const AttackContext& context, int active_frames, int attack_power)
{
    AttackVectors vectors = attack_vectors(context.direction);
    bn::fixed_point center = context.position + vectors.forward;

    _areas[0].center = center;
    _areas[1].center = center - vectors.side;
    _areas[2].center = center + vectors.side;
    _remaining_frames = active_frames;
    _attack_power = attack_power;
    _hit_target_count = 0;
    _newly_activated = true;

    for(int index = 0; index < area_count; ++index)
    {
        _effect_sprites[index].set_position(_areas[index].center);
        _effect_sprites[index].set_visible(true);
    }
}

void MeleeHitbox::update()
{
    if(! active())
    {
        return;
    }

    if(_newly_activated)
    {
        _newly_activated = false;
        return;
    }

    --_remaining_frames;

    if(! _remaining_frames)
    {
        for(bn::sprite_ptr& sprite : _effect_sprites)
        {
            sprite.set_visible(false);
        }
    }
}

bool MeleeHitbox::active() const
{
    return _remaining_frames > 0;
}

int MeleeHitbox::try_hit(int target_id, const bn::fixed_point& target_center,
                         int target_width, int target_height)
{
    if(! active())
    {
        return 0;
    }

    for(int index = 0; index < _hit_target_count; ++index)
    {
        if(_hit_target_ids[index] == target_id)
        {
            return 0;
        }
    }

    if(_hit_target_count == max_hit_targets)
    {
        return 0;
    }

    for(const AttackArea& area : _areas)
    {
        if(area.intersects(target_center, target_width, target_height))
        {
            _hit_target_ids[_hit_target_count] = target_id;
            ++_hit_target_count;
            return _attack_power;
        }
    }

    return 0;
}
