#ifndef DEBUG_COLLISION_DEBUG_OVERLAY_H
#define DEBUG_COLLISION_DEBUG_OVERLAY_H

#include "bn_array.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

#include "combat/collision/collision_debug_box.h"

class CollisionDebugOverlay
{
public:
    void reset();
    void toggle();
    void update(const CollisionDebugBoxList& boxes);

    [[nodiscard]] bool enabled() const;

private:
    static constexpr int corners_per_box = 4;
    static constexpr int max_corners = CollisionDebugBoxList::capacity * corners_per_box;

    void _set_corner(int index, CollisionDebugBoxType type,
                     bool horizontal_flip, bool vertical_flip);
    void _clear();

    bn::array<bn::optional<bn::sprite_ptr>, max_corners> _corners = {};
    bn::array<CollisionDebugBoxType, max_corners> _corner_types = {};
    bool _enabled = false;
};

#endif
