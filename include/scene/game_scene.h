#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "character/player_controller.h"
#include "character/swordsman.h"
#include "character/training_dummy_manager.h"
#include "world/battlefield.h"

class GameScene
{
public:
    GameScene();

    void enter();
    void update();

private:
    Battlefield _battlefield;
    Swordsman _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    TrainingDummyManager _training_dummies;
};

#endif
