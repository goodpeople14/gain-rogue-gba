#ifndef GAME_H
#define GAME_H

#include "game/game_state.h"
#include "game/game_session.h"
#include "scene/game_scene.h"
#include "scene/title_scene.h"

class Game
{
public:
    Game();

    void update();

private:
    GameState _state;
    GameSession _session;
    TitleScene _title_scene;
    GameScene _game_scene;
};

#endif
