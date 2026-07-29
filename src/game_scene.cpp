#include "game_scene.h"

#include "bn_bg_palettes.h"
#include "bn_keypad.h"

#include "bn_sprite_items_swordsman_8dir_sheet.h"

namespace
{
    constexpr bn::color game_background_color(3, 12, 7);
    constexpr int player_size = 16;
    constexpr int player_start_x = 0;
    constexpr int player_start_y = 0;
    constexpr bn::fixed player_movement_speed(1);

    constexpr MovementBounds player_bounds = Battlefield::movement_bounds(player_size, player_size);

    static_assert(player_bounds.min_x == -72 && player_bounds.max_x == 72);
    static_assert(player_bounds.min_y == -72 && player_bounds.max_y == 72);
}

GameScene::GameScene() :
    _player(bn::sprite_items::swordsman_8dir_sheet, player_start_x, player_start_y,
            Direction::DOWN, player_movement_speed, _player_attack),
    _player_bounds(player_bounds)
{
    _player.set_visible(false);
}

void GameScene::enter()
{
    bn::bg_palettes::set_transparent_color(game_background_color);
    _battlefield.set_visible(true);
    _player.set_visible(true);
    _training_dummies.enter();
}

void GameScene::update()
{
    _player_controller.update(_player, _player_bounds);

    if(bn::keypad::a_pressed())
    {
        _player.attack();
    }

    _player.update_attack();
    _training_dummies.update(_player.position());
    _training_dummies.resolve_attack(_player_attack);
}
