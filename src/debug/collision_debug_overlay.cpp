#include "debug/collision_debug_overlay.h"

#include "bn_sprite_items_collision_debug_hit_corner.h"
#include "bn_sprite_items_collision_debug_hurt_corner.h"
#include "bn_sprite_items_collision_debug_push_corner.h"

namespace
{
    constexpr int corner_half_size = 4;

    [[nodiscard]] const bn::sprite_item& corner_item(CollisionDebugBoxType type)
    {
        switch(type)
        {
        case CollisionDebugBoxType::HURTBOX:
            return bn::sprite_items::collision_debug_hurt_corner;
        case CollisionDebugBoxType::HITBOX:
            return bn::sprite_items::collision_debug_hit_corner;
        case CollisionDebugBoxType::PUSHBOX:
            return bn::sprite_items::collision_debug_push_corner;
        default:
            return bn::sprite_items::collision_debug_hurt_corner;
        }
    }
}

void CollisionDebugOverlay::reset()
{
    _enabled = false;
    _clear();
}

void CollisionDebugOverlay::toggle()
{
    _enabled = ! _enabled;
    if(! _enabled)
    {
        _clear();
    }
}

void CollisionDebugOverlay::update(const CollisionDebugBoxList& boxes)
{
    if(! _enabled)
    {
        return;
    }

    int corner_index = 0;
    for(int index = 0; index < boxes.count(); ++index)
    {
        const CollisionDebugBox& debug_box = boxes.boxes()[index];
        const WorldBox& box = debug_box.box;
        bn::fixed half_width = bn::fixed(box.width) / 2;
        bn::fixed half_height = bn::fixed(box.height) / 2;
        bn::fixed left = box.center.x() - half_width;
        bn::fixed right = box.center.x() + half_width;
        bn::fixed top = box.center.y() - half_height;
        bn::fixed bottom = box.center.y() + half_height;

        _set_corner(corner_index++, debug_box.type, false, false);
        _corners[corner_index - 1]->set_position(left + corner_half_size, top + corner_half_size);
        _set_corner(corner_index++, debug_box.type, true, false);
        _corners[corner_index - 1]->set_position(right - corner_half_size, top + corner_half_size);
        _set_corner(corner_index++, debug_box.type, false, true);
        _corners[corner_index - 1]->set_position(left + corner_half_size, bottom - corner_half_size);
        _set_corner(corner_index++, debug_box.type, true, true);
        _corners[corner_index - 1]->set_position(right - corner_half_size, bottom - corner_half_size);
    }

    for(; corner_index < max_corners; ++corner_index)
    {
        _corners[corner_index].reset();
    }
}

bool CollisionDebugOverlay::enabled() const
{
    return _enabled;
}

void CollisionDebugOverlay::_set_corner(int index, CollisionDebugBoxType type,
                                        bool horizontal_flip, bool vertical_flip)
{
    bn::optional<bn::sprite_ptr>& corner = _corners[index];
    if(! corner)
    {
        corner = corner_item(type).create_sprite(0, 0);
        corner->set_z_order(-3);
        _corner_types[index] = type;
    }
    else if(_corner_types[index] != type)
    {
        corner->set_item(corner_item(type));
        _corner_types[index] = type;
    }

    corner->set_horizontal_flip(horizontal_flip);
    corner->set_vertical_flip(vertical_flip);
}

void CollisionDebugOverlay::_clear()
{
    for(bn::optional<bn::sprite_ptr>& corner : _corners)
    {
        corner.reset();
    }
}
