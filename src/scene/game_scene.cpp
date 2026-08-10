#include "scene/game_scene.h"

#include "bn_bg_palettes.h"
#include "bn_keypad.h"

#include "combat/collision/movement_collision.h"
#include "world/stages/stage1.h"

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
    // The home must leave the existing 2px respawn clearance outside Stage1's bottom BLOCKED cell.
    constexpr bn::fixed_point crossbow_goblin_home_position(0, 60);
    constexpr int crossbow_goblin_target_id = 14;
    constexpr int movement_query_padding = 1;
    constexpr bn::array<SpatialActorId, GameScene::goblin_count> goblin_spatial_actor_ids = {{
        SpatialActorId::GOBLIN_0, SpatialActorId::GOBLIN_1,
        SpatialActorId::GOBLIN_2, SpatialActorId::GOBLIN_3
    }};

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
    static_assert(GameScene::enemy_count + 1 == SpatialManager::actor_count);
    static_assert(2 + max_hitboxes_per_frame + (GameScene::goblin_count * 3) + 8 +
                  CrossbowProjectilePool::capacity == CollisionDebugBoxList::capacity);
}

GameScene::GameScene() :
    _player(bn::fixed_point(player_start_x, player_start_y)),
    _player_bounds(player_bounds),
    _goblins{{
        Goblin(goblin_home_positions[0], goblin_target_ids[0]),
        Goblin(goblin_home_positions[1], goblin_target_ids[1]),
        Goblin(goblin_home_positions[2], goblin_target_ids[2]),
        Goblin(goblin_home_positions[3], goblin_target_ids[3])
    }},
    _crossbow_goblin(crossbow_goblin_home_position, crossbow_goblin_target_id),
    _spatial_debug_overlay(stage1::data),
    _spatial_manager(stage1::data)
{
    _player.set_visible(false);
    for(Goblin& goblin : _goblins)
    {
        goblin.set_visible(false);
    }
    _crossbow_goblin.set_visible(false);
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
    _crossbow_goblin.enter();
    _crossbow_projectiles.clear();
    _hit_effects.clear();
    _collision_debug_overlay.reset();
    _spatial_debug_overlay.reset();
    _sync_spatial_actors();
}

void GameScene::update()
{
    PlayerCommand command = _player_controller.command(_player);
    const MovementIntent& movement = command.movement;

    if(movement.moving)
    {
        bn::fixed_point resolved_position = resolve_movement(
                _player.position(), movement.delta, _player.collision_body().pushbox,
                _spatial_manager.movement_obstacles(
                        SpatialActorId::PLAYER, _movement_query_area(world_box(
                                _player.position(), _player.collision_body().pushbox.box))),
                _player_bounds);
        _player.apply_movement(resolved_position, movement.direction);
    }

    _sync_spatial_actor(SpatialActorId::PLAYER,
                        world_box(_player.position(), _player.collision_body().pushbox.box), true);

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    for(int index = 0; index < goblin_count; ++index)
    {
        Goblin& goblin = _goblins[index];
        goblin.update(player_hurtbox, player_pushbox, _spatial_manager.movement_obstacles(
                goblin_spatial_actor_ids[index], _movement_query_area(goblin.movement_obstacle_query_area())));
        _sync_spatial_actor(goblin_spatial_actor_ids[index], goblin.world_pushbox(), goblin.active());
    }
    _crossbow_goblin.update(player_hurtbox, player_pushbox, _spatial_manager.movement_obstacles(
            SpatialActorId::CROSSBOW_GOBLIN,
            _movement_query_area(_crossbow_goblin.movement_obstacle_query_area())), _crossbow_projectiles);
    _sync_spatial_actor(SpatialActorId::CROSSBOW_GOBLIN, _crossbow_goblin.world_pushbox(), _crossbow_goblin.active());
    _crossbow_projectiles.update();

    for(Goblin& goblin : _goblins)
    {
        goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
        goblin.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    }
    _crossbow_goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
    _crossbow_projectiles.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    _sync_spatial_actors();
    _hit_effects.update();

    if(bn::keypad::select_pressed())
    {
        _collision_debug_overlay.toggle();
        _spatial_debug_overlay.set_visible(_collision_debug_overlay.enabled());
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
    _crossbow_goblin.append_collision_debug_boxes(player_hurtbox, boxes);
    _crossbow_projectiles.append_collision_debug_boxes(boxes);
    _collision_debug_overlay.update(boxes);
}

void GameScene::_sync_spatial_actors()
{
    _sync_spatial_actor(SpatialActorId::PLAYER,
                        world_box(_player.position(), _player.collision_body().pushbox.box), true);
    for(int index = 0; index < goblin_count; ++index)
    {
        const Goblin& goblin = _goblins[index];
        _sync_spatial_actor(goblin_spatial_actor_ids[index], goblin.world_pushbox(), goblin.active());
    }
    _sync_spatial_actor(SpatialActorId::CROSSBOW_GOBLIN,
                        _crossbow_goblin.world_pushbox(), _crossbow_goblin.active());
}

void GameScene::_sync_spatial_actor(SpatialActorId actor_id, const WorldBox& pushbox, bool active)
{
    _spatial_manager.update_actor(actor_id, pushbox);
    _spatial_manager.set_actor_active(actor_id, active);
}

WorldBox GameScene::_movement_query_area(const WorldBox& pushbox)
{
    return { pushbox.center, pushbox.width + movement_query_padding * 2,
             pushbox.height + movement_query_padding * 2 };
}
