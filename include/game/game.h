#ifndef GAME_H
#define GAME_H

#include "game/game_state.h"
#include "game/game_session.h"
#include "audio/audio_system.h"
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
    AudioSystem _audio;
    TitleScene _title_scene;
    GameScene _game_scene;
};

#endif
