#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "bn_array.h"

#include "character/player_controller.h"
#include "character/swordsman.h"
#include "character/goblin.h"
#include "combat/hit_effect_manager.h"
#include "debug/collision_debug_overlay.h"
#include "world/battlefield.h"

class GameScene
{
public:
    static constexpr int goblin_count = 4;

    GameScene();

    void enter();
    void update();

private:
    void _update_collision_debug_overlay();
    [[nodiscard]] WorldBoxList<goblin_count> _active_goblin_pushboxes() const;
    [[nodiscard]] WorldBoxList<max_movement_obstacles> _goblin_blocking_pushboxes(
            int goblin_index, const WorldBox& player_pushbox) const;

    Battlefield _battlefield;
    Swordsman _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    bn::array<Goblin, goblin_count> _goblins;
    HitEffectManager _hit_effects;
    CollisionDebugOverlay _collision_debug_overlay;
};

#endif
