#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "bn_array.h"

#include "character/player_controller.h"
#include "character/swordsman.h"
#include "character/goblin.h"
#include "character/crossbow_goblin.h"
#include "combat/crossbow_projectile_pool.h"
#include "combat/hit_effect_manager.h"
#include "debug/collision_debug_overlay.h"
#include "world/battlefield.h"

class GameScene
{
public:
    static constexpr int goblin_count = 4;
    static constexpr int crossbow_goblin_count = 1;
    static constexpr int enemy_count = goblin_count + crossbow_goblin_count;

    GameScene();

    void enter();
    void update();

private:
    void _update_collision_debug_overlay();
    [[nodiscard]] WorldBoxList<enemy_count> _active_enemy_pushboxes() const;
    [[nodiscard]] WorldBoxList<max_movement_obstacles> _goblin_blocking_pushboxes(
            int goblin_index, const WorldBox& player_pushbox) const;
    [[nodiscard]] WorldBoxList<max_movement_obstacles> _crossbow_blocking_pushboxes(
            const WorldBox& player_pushbox) const;

    Battlefield _battlefield;
    Swordsman _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    bn::array<Goblin, goblin_count> _goblins;
    CrossbowGoblin _crossbow_goblin;
    CrossbowProjectilePool _crossbow_projectiles;
    HitEffectManager _hit_effects;
    CollisionDebugOverlay _collision_debug_overlay;
};

#endif
