#ifndef DEBUG_COLLISION_DEBUG_OVERLAY_H
#define DEBUG_COLLISION_DEBUG_OVERLAY_H

#include "bn_array.h"
#include "bn_optional.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"

#include "combat/collision/collision_debug_box.h"

class CollisionDebugOverlay
{
public:
    void reset();
    void clear();
    void toggle();
    void update(const CollisionDebugBoxList& boxes, const CollisionDebugRadiusList& radii);

    [[nodiscard]] bool enabled() const;

private:
    // Four Crossbow radius guides require 44 fixed non-affine segments. The
    // bounded physical boxes contribute at most four segments each.
    static constexpr int max_radius_guide_segments = 44;
    static constexpr int max_box_segments = CollisionDebugBoxList::capacity * 4;
    static constexpr int max_segments = max_radius_guide_segments + max_box_segments;
    static_assert(max_segments == 84);

    void _set_segment(int index, const bn::sprite_item& item, const bn::fixed_point& position);
    void _clear();

    bn::array<bn::optional<bn::sprite_ptr>, max_segments> _segments = {};
    bool _enabled = false;
};

#endif
