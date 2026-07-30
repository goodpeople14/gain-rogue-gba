#include "scene/game_scene.h"

#include "bn_bg_palettes.h"

#include "combat/collision/movement_collision.h"

namespace
{
    constexpr bn::color game_background_color(3, 12, 7);
    constexpr int player_size = 16;
    constexpr int player_start_x = 0;
    constexpr int player_start_y = 0;

    constexpr MovementBounds player_bounds = Battlefield::movement_bounds(player_size, player_size);

    static_assert(player_bounds.min_x == -72 && player_bounds.max_x == 72);
    static_assert(player_bounds.min_y == -72 && player_bounds.max_y == 72);
}

GameScene::GameScene() :
    _player(bn::fixed_point(player_start_x, player_start_y)),
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
    PlayerCommand command = _player_controller.command(_player);
    const MovementIntent& movement = command.movement;

    if(movement.moving)
    {
        bn::fixed_point resolved_position = resolve_movement(
                _player.position(), movement.delta, _player.collision_body().pushbox,
                _training_dummies.active_pushboxes(), _player_bounds);
        _player.apply_movement(resolved_position, movement.direction);
    }

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    _training_dummies.update(player_pushbox);
    _training_dummies.resolve_attack(_player.melee_attack());
}
