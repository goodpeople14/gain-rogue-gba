#include "title_scene.h"

#include "bn_array.h"
#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

namespace
{
    constexpr bn::tile make_glyph(const bn::array<unsigned char, 7>& rows)
    {
        bn::tile result = {};

        for(int row = 0; row < 7; ++row)
        {
            unsigned int pixels = 0;

            for(int column = 0; column < 5; ++column)
            {
                if(rows[row] & (1 << (4 - column)))
                {
                    pixels |= 1U << ((column + 1) * 4);
                }
            }

            result.data[row] = pixels;
        }

        return result;
    }

    constexpr bn::array<bn::tile, 11> glyph_tiles = {
        make_glyph({ 14, 17, 17, 31, 17, 17, 17 }),  // A
        make_glyph({ 31, 16, 16, 30, 16, 16, 31 }),  // E
        make_glyph({ 14, 17, 16, 23, 17, 17, 14 }),  // G
        make_glyph({ 31, 4, 4, 4, 4, 4, 31 }),       // I
        make_glyph({ 17, 25, 21, 19, 17, 17, 17 }),  // N
        make_glyph({ 14, 17, 17, 17, 17, 17, 14 }),  // O
        make_glyph({ 30, 17, 17, 30, 16, 16, 16 }),  // P
        make_glyph({ 30, 17, 17, 30, 20, 18, 17 }),  // R
        make_glyph({ 15, 16, 16, 14, 1, 1, 30 }),    // S
        make_glyph({ 31, 4, 4, 4, 4, 4, 4 }),        // T
        make_glyph({ 17, 17, 17, 17, 17, 17, 14 })   // U
    };

    constexpr bn::array<bn::color, 16> glyph_colors = {
        bn::color(0, 0, 0), bn::color(31, 28, 16), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::sprite_item glyph_item(
            bn::sprite_shape_size(8, 8), glyph_tiles, glyph_colors, bn::bpp_mode::BPP_4, glyph_tiles.size());

    int glyph_index(char character)
    {
        switch(character)
        {
        case 'A': return 0;
        case 'E': return 1;
        case 'G': return 2;
        case 'I': return 3;
        case 'N': return 4;
        case 'O': return 5;
        case 'P': return 6;
        case 'R': return 7;
        case 'S': return 8;
        case 'T': return 9;
        case 'U': return 10;
        default: return -1;
        }
    }

    void add_text(const char* text, int x, int y, int spacing, bn::fixed scale,
                  bn::vector<bn::sprite_ptr, 32>& sprites)
    {
        for(const char* character = text; *character; ++character)
        {
            int index = glyph_index(*character);

            if(index >= 0)
            {
                bn::sprite_ptr sprite = glyph_item.create_sprite(x, y, index);
                sprite.set_scale(scale);
                sprites.push_back(bn::move(sprite));
            }

            x += spacing;
        }
    }
}

TitleScene::TitleScene()
{
    add_text("GAIN ROGUE", -63, -24, 14, 2, _sprites);
    add_text("PRESS START", -40, 28, 8, 1, _sprites);
}

bool TitleScene::update() const
{
    return bn::keypad::start_pressed();
}

void TitleScene::hide()
{
    _sprites.clear();
}
