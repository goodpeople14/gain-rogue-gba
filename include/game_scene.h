#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "battlefield.h"
#include "character.h"
#include "combat/melee/swordsman_attack.h"
#include "combat/training_dummy_manager.h"
#include "player_controller.h"

class GameScene
{
public:
    GameScene();

    void enter();
    void update();

private:
    Battlefield _battlefield;
    SwordsmanAttack _player_attack;
    Character _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    TrainingDummyManager _training_dummies;
};

#endif
