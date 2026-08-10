#ifndef DEBUG_SPATIAL_DEBUG_OVERLAY_H
#define DEBUG_SPATIAL_DEBUG_OVERLAY_H

#include "bn_array.h"
#include "bn_optional.h"
#include "bn_regular_bg_map_cell.h"
#include "bn_regular_bg_ptr.h"

#include "world/stage_data.h"

class SpatialDebugOverlay
{
public:
    explicit SpatialDebugOverlay(const StageData& stage);

    void reset();
    void set_visible(bool visible);

    [[nodiscard]] bool visible() const;

private:
    static constexpr int map_width = 32;
    static constexpr int map_height = 32;

    const StageData& _stage;
    bn::array<bn::regular_bg_map_cell, map_width * map_height> _map_cells = {};
    bn::optional<bn::regular_bg_ptr> _background;
};

#endif
