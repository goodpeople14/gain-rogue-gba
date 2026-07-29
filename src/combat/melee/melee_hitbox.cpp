#include "combat/melee/melee_hitbox.h"

#include "bn_color.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

#include "combat/collision/collision_math.h"
#include "combat/melee/swordsman_attack_data.h"

namespace
{
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

MeleeHitbox::MeleeHitbox() :
    _effect_sprites {
        effect_item.create_sprite(0, 0),
        effect_item.create_sprite(0, 0)
    }
{
    for(bn::sprite_ptr& sprite : _effect_sprites)
    {
        sprite.set_scale(0.5);
        sprite.set_visible(false);
    }
}

void MeleeHitbox::activate(const AttackContext& context, int active_frames, int attack_power)
{
    _context = context;
    _game_frame = 0;
    _total_frames = active_frames;
    _attack_power = attack_power;
    _hit_registry.reset();
    _refresh_effects();
}

void MeleeHitbox::update()
{
    if(_game_frame == 0)
    {
        if(_total_frames > 0)
        {
            _game_frame = 1;
            _refresh_effects();
        }

        return;
    }

    if(_game_frame < _total_frames)
    {
        ++_game_frame;
        _refresh_effects();
    }
    else
    {
        _game_frame = 0;
        _total_frames = 0;
        _refresh_effects();
    }
}

bool MeleeHitbox::active() const
{
    return _game_frame > 0 && _game_frame <= _total_frames;
}

int MeleeHitbox::try_hit(int target_id, const bn::fixed_point& target_position,
                         const Hurtbox& target_hurtbox)
{
    if(! active() || _hit_registry.contains(target_id))
    {
        return 0;
    }

    const AttackFrameData& frame_data = swordsman_attack_frame_data(_context.direction, _game_frame);
    WorldBox hurtbox = world_box(target_position, target_hurtbox.box);

    for(int index = 0; index < frame_data.hitbox_count; ++index)
    {
        WorldBox hitbox = world_box(_context.position, frame_data.hitboxes[index].box);

        if(touches_or_intersects(hitbox, hurtbox))
        {
            _hit_registry.add(target_id);
            return _attack_power;
        }
    }

    return 0;
}

void MeleeHitbox::_refresh_effects()
{
    int hitbox_count = 0;

    if(active())
    {
        const AttackFrameData& frame_data = swordsman_attack_frame_data(_context.direction, _game_frame);
        hitbox_count = frame_data.hitbox_count;

        for(int index = 0; index < hitbox_count; ++index)
        {
            _effect_sprites[index].set_position(
                    world_box(_context.position, frame_data.hitboxes[index].box).center);
            _effect_sprites[index].set_visible(true);
        }
    }

    for(int index = hitbox_count; index < effect_sprite_count; ++index)
    {
        _effect_sprites[index].set_visible(false);
    }
}
