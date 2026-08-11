#include "world/battlefield.h"

#include "bn_regular_bg_items_stage1_terrain.h"
#include "bn_regular_bg_items_stage2_terrain.h"

Battlefield::Battlefield() :
    _stage1_background(bn::regular_bg_items::stage1_terrain.create_bg(0, 0)),
    _stage2_background(bn::regular_bg_items::stage2_terrain.create_bg(0, 0))
{
    // SpatialDebugOverlay is priority 2, so this visual stage layer stays behind it.
    _stage1_background.set_priority(3);
    _stage2_background.set_priority(3);
    _stage1_background.set_visible(false);
    _stage2_background.set_visible(false);
}

void Battlefield::set_visible(bool visible)
{
    _stage1_background.set_visible(visible && _stage == StageVisual::STAGE_1);
    _stage2_background.set_visible(visible && _stage == StageVisual::STAGE_2);
}

void Battlefield::set_stage(StageVisual stage)
{
    _stage = stage;
    set_visible(true);
}
