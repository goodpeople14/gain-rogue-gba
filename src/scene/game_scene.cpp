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
    constexpr bn::array<bn::fixed_point, GameScene::goblin_count> goblin_home_positions = {{
        { -40, -32 }, { 40, -32 }, { -40, 32 }, { 40, 32 }
    }};
    constexpr bn::array<int, GameScene::goblin_count> goblin_target_ids = {{ 10, 11, 12, 13 }};

    constexpr MovementBounds player_bounds = Battlefield::movement_bounds(player_size, player_size);

    [[nodiscard]] constexpr bool goblin_target_ids_are_unique()
    {
        for(int first = 0; first < GameScene::goblin_count; ++first)
        {
            if(goblin_target_ids[first] == 0)
            {
                return false;
            }

            for(int second = first + 1; second < GameScene::goblin_count; ++second)
            {
                if(goblin_target_ids[first] == goblin_target_ids[second])
                {
                    return false;
                }
            }
        }

        return true;
    }

    static_assert(player_bounds.min_x == -72 && player_bounds.max_x == 72);
    static_assert(player_bounds.min_y == -72 && player_bounds.max_y == 72);
    static_assert(goblin_target_ids_are_unique());
    static_assert(2 + max_hitboxes_per_frame + (GameScene::goblin_count * 3) == CollisionDebugBoxList::capacity);
}

GameScene::GameScene() :
    _player(bn::fixed_point(player_start_x, player_start_y)),
    _player_bounds(player_bounds),
    _goblins{{
        Goblin(goblin_home_positions[0], goblin_target_ids[0]),
        Goblin(goblin_home_positions[1], goblin_target_ids[1]),
        Goblin(goblin_home_positions[2], goblin_target_ids[2]),
        Goblin(goblin_home_positions[3], goblin_target_ids[3])
    }}
{
    _player.set_visible(false);
    for(Goblin& goblin : _goblins)
    {
        goblin.set_visible(false);
    }
}

void GameScene::enter()
{
    bn::bg_palettes::set_transparent_color(game_background_color);
    _battlefield.set_visible(true);
    _player.set_visible(true);
    for(Goblin& goblin : _goblins)
    {
        goblin.enter();
    }
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
                _active_goblin_pushboxes(), _player_bounds);
        _player.apply_movement(resolved_position, movement.direction);
    }

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    for(int index = 0; index < goblin_count; ++index)
    {
        _goblins[index].update(player_hurtbox, player_pushbox, _goblin_blocking_pushboxes(index, player_pushbox));
    }

    for(Goblin& goblin : _goblins)
    {
        goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
        goblin.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    }
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
    for(const Goblin& goblin : _goblins)
    {
        goblin.append_collision_debug_boxes(player_hurtbox, boxes);
    }
    _collision_debug_overlay.update(boxes);
}

WorldBoxList<GameScene::goblin_count> GameScene::_active_goblin_pushboxes() const
{
    WorldBoxList<goblin_count> result;

    for(const Goblin& goblin : _goblins)
    {
        if(goblin.active())
        {
            result.boxes[result.count] = goblin.world_pushbox();
            ++result.count;
        }
    }

    return result;
}

WorldBoxList<max_movement_obstacles> GameScene::_goblin_blocking_pushboxes(
        int goblin_index, const WorldBox& player_pushbox) const
{
    WorldBoxList<max_movement_obstacles> result;
    result.boxes[result.count] = player_pushbox;
    ++result.count;

    for(int index = 0; index < goblin_count; ++index)
    {
        if(index != goblin_index && _goblins[index].active())
        {
            result.boxes[result.count] = _goblins[index].world_pushbox();
            ++result.count;
        }
    }

    return result;
}
