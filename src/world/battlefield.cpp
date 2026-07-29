#include "world/battlefield.h"

#include "bn_array.h"
#include "bn_color.h"
#include "bn_regular_bg_item.h"
#include "bn_tile.h"

namespace
{
    constexpr bn::array<bn::tile, 2> battlefield_tiles = {
        bn::tile { { 0, 0, 0, 0, 0, 0, 0, 0 } },
        bn::tile { { 0x11111111, 0x11111111, 0x11111111, 0x11111111,
                     0x11111111, 0x11111111, 0x11111111, 0x11111111 } }
    };

    constexpr bn::array<bn::color, 16> battlefield_colors = {
        bn::color(0, 0, 0), bn::color(7, 8, 11), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::array<bn::regular_bg_map_cell, 32 * 32> make_battlefield_map()
    {
        bn::array<bn::regular_bg_map_cell, 32 * 32> result = {};

        // A centered 256px background shows columns 1 through 30 on the 240px screen.
        // Five 8px columns on each side form the two 40px UI panels.
        for(int row = 0; row < 32; ++row)
        {
            for(int column = 1; column <= 30; ++column)
            {
                if(column <= 5 || column >= 26)
                {
                    result[(row * 32) + column] = 1;
                }
            }
        }

        return result;
    }

    constexpr bn::array<bn::regular_bg_map_cell, 32 * 32> battlefield_map = make_battlefield_map();

    constexpr bn::regular_bg_item battlefield_item(
            battlefield_tiles, battlefield_colors, bn::bpp_mode::BPP_4,
            battlefield_map[0], bn::size(32, 32));
}

Battlefield::Battlefield() :
    _background(battlefield_item.create_bg(0, 0))
{
    _background.set_visible(false);
}

void Battlefield::set_visible(bool visible)
{
    _background.set_visible(visible);
}
