#include "game.h"

#include "bn_bg_palettes.h"

namespace
{
    constexpr bn::color title_background_color(2, 4, 9);
}

Game::Game() :
    _state(GameState::TITLE)
{
    bn::bg_palettes::set_transparent_color(title_background_color);
}

void Game::update()
{
    if(_state == GameState::TITLE)
    {
        if(_title_scene.update())
        {
            _title_scene.hide();
            _game_scene.enter();
            _state = GameState::GAME;
        }
    }
    else
    {
        _game_scene.update();
    }
}
