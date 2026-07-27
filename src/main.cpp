#include "bn_core.h"
#include "bn_array.h"
#include "bn_bg_palettes.h"
#include "bn_keypad.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"
#include "bn_tile.h"
#include "bn_vector.h"

#include "bn_sprite_items_gunner_8dir_sheet.h"

namespace
{
    enum class game_state
    {
        TITLE,
        GAME
    };

    enum class player_direction
    {
        DOWN = 0,
        DOWN_LEFT = 1,
        LEFT = 2,
        UP_LEFT = 3,
        UP = 4,
        UP_RIGHT = 5,
        RIGHT = 6,
        DOWN_RIGHT = 7
    };

    constexpr bn::color title_background_color(2, 4, 9);
    constexpr bn::color game_background_color(3, 12, 7);

    constexpr int screen_width = 240;
    constexpr int screen_height = 160;
    constexpr int ui_panel_width = 40;
    constexpr int battlefield_width = screen_width - (ui_panel_width * 2);
    constexpr int battlefield_height = screen_height;
    constexpr int player_size = 16;
    constexpr int player_half_size = player_size / 2;
    constexpr int player_min_x = -(battlefield_width / 2) + player_half_size;
    constexpr int player_max_x = (battlefield_width / 2) - player_half_size;
    constexpr int player_min_y = -(battlefield_height / 2) + player_half_size;
    constexpr int player_max_y = (battlefield_height / 2) - player_half_size;
    constexpr int player_start_x = 0;
    constexpr int player_start_y = 0;
    constexpr bn::fixed player_movement_speed(1);
    constexpr bn::fixed player_diagonal_movement_speed(0.70710678f);

    static_assert(player_min_x == -72 && player_max_x == 72);
    static_assert(player_min_y == -72 && player_max_y == 72);

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

    bn::fixed clamp_player_position(bn::fixed position, int minimum, int maximum)
    {
        if(position < minimum)
        {
            return minimum;
        }

        if(position > maximum)
        {
            return maximum;
        }

        return position;
    }

    player_direction direction_from_input(int horizontal, int vertical)
    {
        if(vertical > 0)
        {
            if(horizontal < 0)
            {
                return player_direction::DOWN_LEFT;
            }

            if(horizontal > 0)
            {
                return player_direction::DOWN_RIGHT;
            }

            return player_direction::DOWN;
        }

        if(vertical < 0)
        {
            if(horizontal < 0)
            {
                return player_direction::UP_LEFT;
            }

            if(horizontal > 0)
            {
                return player_direction::UP_RIGHT;
            }

            return player_direction::UP;
        }

        return horizontal < 0 ? player_direction::LEFT : player_direction::RIGHT;
    }

    void update_player(bn::sprite_ptr& player, player_direction& direction)
    {
        int horizontal = int(bn::keypad::right_held()) - int(bn::keypad::left_held());
        int vertical = int(bn::keypad::down_held()) - int(bn::keypad::up_held());

        if(horizontal || vertical)
        {
            direction = direction_from_input(horizontal, vertical);
            player.set_tiles(bn::sprite_items::gunner_8dir_sheet.tiles_item(), int(direction));

            bn::fixed movement_speed = horizontal && vertical ?
                    player_diagonal_movement_speed : player_movement_speed;

            player.set_position(
                    clamp_player_position(player.x() + (horizontal * movement_speed), player_min_x, player_max_x),
                    clamp_player_position(player.y() + (vertical * movement_speed), player_min_y, player_max_y));
        }
    }

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

int main()
{
    bn::core::init();

    game_state state = game_state::TITLE;
    bn::bg_palettes::set_transparent_color(title_background_color);

    bn::vector<bn::sprite_ptr, 32> title_sprites;
    add_text("GAIN ROGUE", -63, -24, 14, 2, title_sprites);
    add_text("PRESS START", -40, 28, 8, 1, title_sprites);

    bn::regular_bg_ptr battlefield = battlefield_item.create_bg(0, 0);
    battlefield.set_visible(false);

    player_direction player_facing = player_direction::DOWN;
    bn::sprite_ptr player = bn::sprite_items::gunner_8dir_sheet.create_sprite(
            player_start_x, player_start_y, int(player_facing));
    player.set_visible(false);

    while(true)
    {
        if(state == game_state::TITLE && bn::keypad::start_pressed())
        {
            title_sprites.clear();
            bn::bg_palettes::set_transparent_color(game_background_color);
            battlefield.set_visible(true);
            player.set_visible(true);
            state = game_state::GAME;
        }
        else if(state == game_state::GAME)
        {
            update_player(player, player_facing);
        }

        bn::core::update();
    }
}
