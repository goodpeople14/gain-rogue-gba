#include "scene/game_scene.h"

#include "bn_bg_palettes.h"
#include "bn_keypad.h"

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
    _player_bounds(player_bounds),
    _goblin(bn::fixed_point(-36, 0))
{
    _player.set_visible(false);
    _goblin.set_visible(false);
}

void GameScene::enter()
{
    bn::bg_palettes::set_transparent_color(game_background_color);
    _battlefield.set_visible(true);
    _player.set_visible(true);
    _goblin.enter();
    _hit_effects.clear();
    _collision_debug_overlay.reset();
}

void GameScene::update()
{
    PlayerCommand command = _player_controller.command(_player);
    const MovementIntent& movement = command.movement;

    if(movement.moving)
    {
        bn::fixed_point resolved_position = resolve_movement(
                _player.position(), movement.delta, _player.collision_body().pushbox,
                _goblin.active_pushboxes(), _player_bounds);
        _player.apply_movement(resolved_position, movement.direction);
    }

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    _goblin.update(player_hurtbox, player_pushbox);
    _goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
    _goblin.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    _hit_effects.update();

    if(bn::keypad::select_pressed())
    {
        _collision_debug_overlay.toggle();
    }

    _update_collision_debug_overlay();
}

void GameScene::_update_collision_debug_overlay()
{
    if(! _collision_debug_overlay.enabled())
    {
        return;
    }

    CollisionDebugBoxList boxes;
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    boxes.add(player_hurtbox, CollisionDebugBoxType::HURTBOX);
    boxes.add(world_box(_player.position(), _player.collision_body().pushbox.box), CollisionDebugBoxType::PUSHBOX);
    WorldBoxList<max_hitboxes_per_frame> player_hitboxes = _player.melee_attack().active_hitboxes();
    for(int index = 0; index < player_hitboxes.count; ++index)
    {
        boxes.add(player_hitboxes.boxes[index], CollisionDebugBoxType::HITBOX);
    }
    _goblin.append_collision_debug_boxes(player_hurtbox, boxes);
    _collision_debug_overlay.update(boxes);
}
