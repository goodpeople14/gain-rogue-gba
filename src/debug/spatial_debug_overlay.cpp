#include "debug/spatial_debug_overlay.h"

#include "bn_assert.h"
#include "bn_color.h"
#include "bn_regular_bg_item.h"
#include "bn_tile.h"

#include "world/stages/stage1.h"

namespace
{
    constexpr int debug_tile_size = 8;
    constexpr int transparent_tile_index = 0;
    constexpr int blocked_tile_index = 1;

    constexpr bn::array<bn::tile, 2> debug_tiles = {
        bn::tile { { 0, 0, 0, 0, 0, 0, 0, 0 } },
        bn::tile { { 0x10001000, 0x01000100, 0x00100010, 0x00010001,
                     0x10001000, 0x01000100, 0x00100010, 0x00010001 } }
    };

    constexpr bn::array<bn::color, 16> debug_colors = {
        bn::color(0, 0, 0), bn::color(31, 0, 31), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    [[nodiscard]] constexpr int tile_index(StageCell cell)
    {
        return cell == StageCell::BLOCKED ? blocked_tile_index : transparent_tile_index;
    }

    [[nodiscard]] constexpr int stage_map_offset(int map_size, int stage_size)
    {
        return (map_size - stage_size) / 2;
    }

    template<int Width, int Height>
    [[nodiscard]] constexpr bn::array<bn::regular_bg_map_cell, Width * Height> make_debug_map(const StageData& stage)
    {
        bn::array<bn::regular_bg_map_cell, Width * Height> result = {};
        int offset_x = stage_map_offset(Width, stage.width);
        int offset_y = stage_map_offset(Height, stage.height);

        for(int cell_y = 0; cell_y < stage.height; ++cell_y)
        {
            for(int cell_x = 0; cell_x < stage.width; ++cell_x)
            {
                int map_index = ((offset_y + cell_y) * Width) + offset_x + cell_x;
                result[map_index] = tile_index(stage_cell_at(stage, cell_x, cell_y));
            }
        }

        return result;
    }

    constexpr bn::array<bn::regular_bg_map_cell, 32 * 32> stage1_debug_map = make_debug_map<32, 32>(stage1::data);

    static_assert(tile_index(StageCell::WALKABLE) == transparent_tile_index);
    static_assert(tile_index(StageCell::BLOCKED) == blocked_tile_index);
    static_assert(stage_map_offset(32, stage1::width) == 6);
    static_assert(stage1_debug_map[(6 * 32) + 6] == blocked_tile_index);
    static_assert(stage1_debug_map[(7 * 32) + 7] == transparent_tile_index);
    static_assert(stage1_debug_map[(12 * 32) + 13] == blocked_tile_index);
    static_assert(stage1_debug_map[(25 * 32) + 25] == blocked_tile_index);
}

SpatialDebugOverlay::SpatialDebugOverlay(const StageData& stage) : _stage(stage)
{
    BN_ASSERT(stage.width <= map_width);
    BN_ASSERT(stage.height <= map_height);
    BN_ASSERT(stage.tile_size == debug_tile_size);

    _map_cells = make_debug_map<map_width, map_height>(_stage);

    bn::regular_bg_item debug_item(
            debug_tiles, debug_colors, bn::bpp_mode::BPP_4,
            _map_cells[0], bn::size(map_width, map_height));
    _background = debug_item.create_bg(0, 0);
    _background->set_priority(2);
    _background->set_visible(false);
}

void SpatialDebugOverlay::reset()
{
    set_visible(false);
}

void SpatialDebugOverlay::set_visible(bool visible)
{
    _background->set_visible(visible);
}

bool SpatialDebugOverlay::visible() const
{
    return _background->visible();
}
