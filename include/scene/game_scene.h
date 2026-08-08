#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "character/player_controller.h"
#include "character/swordsman.h"
#include "character/goblin.h"
#include "combat/hit_effect_manager.h"
#include "debug/collision_debug_overlay.h"
#include "world/battlefield.h"

class GameScene
{
public:
    GameScene();

    void enter();
    void update();

private:
    void _update_collision_debug_overlay();

    Battlefield _battlefield;
    Swordsman _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    Goblin _goblin;
    HitEffectManager _hit_effects;
    CollisionDebugOverlay _collision_debug_overlay;
};

#endif
