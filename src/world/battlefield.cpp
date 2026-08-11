#include "world/battlefield.h"

#include "bn_regular_bg_items_stage1_terrain.h"

Battlefield::Battlefield() :
    _background(bn::regular_bg_items::stage1_terrain.create_bg(0, 0))
{
    // SpatialDebugOverlay is priority 2, so this visual stage layer stays behind it.
    _background.set_priority(3);
    _background.set_visible(false);
}

void Battlefield::set_visible(bool visible)
{
    _background.set_visible(visible);
}
