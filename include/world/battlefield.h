#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "bn_regular_bg_ptr.h"

struct MovementBounds
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
};

class Battlefield
{
public:
    static constexpr int screen_width = 240;
    static constexpr int screen_height = 160;
    static constexpr int ui_panel_width = 40;
    static constexpr int width = screen_width - (ui_panel_width * 2);
    static constexpr int height = screen_height;

    Battlefield();

    void set_visible(bool visible);

    [[nodiscard]] static constexpr MovementBounds movement_bounds(int character_width, int character_height)
    {
        int half_character_width = character_width / 2;
        int half_character_height = character_height / 2;

        return {
            -(width / 2) + half_character_width,
            (width / 2) - half_character_width,
            -(height / 2) + half_character_height,
            (height / 2) - half_character_height
        };
    }

private:
    bn::regular_bg_ptr _background;
};

#endif
