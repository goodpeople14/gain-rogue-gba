#include "scene/game_scene.h"

#include "bn_bg_palettes.h"
#include "bn_array.h"
#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_log_level.h"
#include "bn_sprite_builder.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

#include "combat/collision/movement_collision.h"
#include "combat/collision/collision_math.h"

#define _goblins (_enemy_runtime.goblins())
#define _crossbow_goblins (_enemy_runtime.crossbow_goblins())

namespace
{
    constexpr bn::color game_background_color(3, 12, 7);
    constexpr int player_size = 16;
    constexpr bool stage_enemy_respawn_enabled = false;
    constexpr int movement_query_padding = 1;
    constexpr int intro_frames = 90;
    constexpr int ready_frames = 120;
    constexpr int go_frames = 45;
    constexpr int congratulations_frames = 120;
    constexpr int game_over_frames = 120;
    constexpr int player_dead_frames = 120;
    constexpr int stage_message_y = 0;
    constexpr int exit_message_y = -65;
    constexpr int player_health_hud_x = -104;
    constexpr int player_health_hud_y = 64;
    constexpr int player_health_hud_spacing = 9;
    constexpr int player_name_hud_spacing = 7;
    constexpr int player_name_to_health_spacing = 5;

    [[nodiscard]] constexpr bool player_damage_applies_in_debug(bool debug_enabled, int damage)
    {
        return damage > 0 && ! debug_enabled;
    }

    [[nodiscard]] constexpr int cycled_debug_enemy_type_index(int current_index, int type_count, int offset)
    {
        int result = (current_index + offset) % type_count;
        return result < 0 ? result + type_count : result;
    }

    void append_debug_enemy_type(
            const Enemy& enemy, bn::vector<CharacterId, GameScene::enemy_count>& types)
    {
        if(! enemy.active())
        {
            return;
        }

        CharacterId type = enemy.definition().id;
        for(CharacterId existing_type : types)
        {
            if(existing_type == type)
            {
                return;
            }
        }

        types.push_back(type);
    }
    constexpr MovementBounds player_bounds = Battlefield::movement_bounds(player_size, player_size);

    constexpr bn::tile make_stage_glyph(const bn::array<unsigned char, 7>& rows)
    {
        bn::tile result = {};

        for(int row = 0; row < 7; ++row)
        {
            unsigned int pixels = 0;

            for(int column = 0; column < 5; ++column)
            {
                if(rows[row] & (1 << (4 - column)))
                {
                    pixels |= 1U << ((column + 1) * 4);
                }
            }

            result.data[row] = pixels;
        }

        return result;
    }

    constexpr bn::array<bn::tile, 23> stage_glyph_tiles = {
        make_stage_glyph({ 14, 17, 17, 31, 17, 17, 17 }),  // A
        make_stage_glyph({ 30, 17, 17, 17, 17, 17, 30 }),  // D
        make_stage_glyph({ 31, 16, 16, 30, 16, 16, 31 }),  // E
        make_stage_glyph({ 14, 17, 16, 23, 17, 17, 14 }),  // G
        make_stage_glyph({ 31, 4, 4, 4, 4, 4, 31 }),       // I
        make_stage_glyph({ 14, 17, 17, 17, 17, 17, 14 }),  // O
        make_stage_glyph({ 30, 17, 17, 30, 20, 18, 17 }),  // R
        make_stage_glyph({ 31, 4, 4, 4, 4, 4, 4 }),        // T
        make_stage_glyph({ 17, 10, 4, 4, 4, 10, 17 }),     // X
        make_stage_glyph({ 17, 17, 10, 4, 4, 4, 4 }),      // Y
        make_stage_glyph({ 4, 4, 4, 4, 4, 0, 4 }),         // !
        make_stage_glyph({ 15, 16, 16, 14, 1, 1, 30 }),    // S
        make_stage_glyph({ 4, 12, 4, 4, 4, 4, 14 }),       // 1
        make_stage_glyph({ 14, 17, 1, 2, 4, 8, 31 }),      // 2
        make_stage_glyph({ 30, 1, 1, 14, 1, 1, 30 }),      // 3
        make_stage_glyph({ 2, 6, 10, 18, 31, 2, 2 }),      // 4
        make_stage_glyph({ 31, 16, 30, 1, 1, 17, 14 }),    // 5
        make_stage_glyph({ 15, 16, 16, 16, 16, 16, 15 }),  // C
        make_stage_glyph({ 16, 16, 16, 16, 16, 16, 31 }),  // L
        make_stage_glyph({ 17, 27, 21, 17, 17, 17, 17 }),  // M
        make_stage_glyph({ 17, 25, 21, 19, 17, 17, 17 }),  // N
        make_stage_glyph({ 17, 17, 17, 17, 17, 17, 14 }),  // U
        make_stage_glyph({ 17, 17, 17, 10, 10, 4, 4 })     // V
    };

    constexpr bn::array<bn::color, 16> stage_glyph_colors = {
        bn::color(0, 0, 0), bn::color(31, 28, 16), bn::color(31, 0, 0), bn::color(12, 0, 0),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::sprite_item stage_glyph_item(
            bn::sprite_shape_size(8, 8), stage_glyph_tiles, stage_glyph_colors,
            bn::bpp_mode::BPP_4, stage_glyph_tiles.size());

    constexpr bn::tile make_player_health_tile(bool filled)
    {
        bn::tile result = {};

        for(int y = 1; y < 7; ++y)
        {
            for(int x = 1; x < 7; ++x)
            {
                if(filled || x == 1 || x == 6 || y == 1 || y == 6)
                {
                    unsigned int color_index = filled ? 2 : 3;
                    result.data[y] |= color_index << (x * 4);
                }
            }
        }

        return result;
    }

    constexpr bn::array<bn::tile, 2> player_health_tile_data = {
        make_player_health_tile(false), make_player_health_tile(true)
    };

    constexpr bn::sprite_tiles_item player_health_tiles_item(
            player_health_tile_data, bn::bpp_mode::BPP_4, player_health_tile_data.size());

    [[nodiscard]] bn::array<bn::sprite_tiles_ptr, stage_glyph_tiles.size()> make_stage_glyph_tiles()
    {
        return {{
            stage_glyph_item.tiles_item().create_tiles(0), stage_glyph_item.tiles_item().create_tiles(1),
            stage_glyph_item.tiles_item().create_tiles(2), stage_glyph_item.tiles_item().create_tiles(3),
            stage_glyph_item.tiles_item().create_tiles(4), stage_glyph_item.tiles_item().create_tiles(5),
            stage_glyph_item.tiles_item().create_tiles(6), stage_glyph_item.tiles_item().create_tiles(7),
            stage_glyph_item.tiles_item().create_tiles(8), stage_glyph_item.tiles_item().create_tiles(9),
            stage_glyph_item.tiles_item().create_tiles(10), stage_glyph_item.tiles_item().create_tiles(11),
            stage_glyph_item.tiles_item().create_tiles(12), stage_glyph_item.tiles_item().create_tiles(13),
            stage_glyph_item.tiles_item().create_tiles(14), stage_glyph_item.tiles_item().create_tiles(15),
            stage_glyph_item.tiles_item().create_tiles(16), stage_glyph_item.tiles_item().create_tiles(17),
            stage_glyph_item.tiles_item().create_tiles(18), stage_glyph_item.tiles_item().create_tiles(19),
            stage_glyph_item.tiles_item().create_tiles(20), stage_glyph_item.tiles_item().create_tiles(21),
            stage_glyph_item.tiles_item().create_tiles(22)
        }};
    }

    [[nodiscard]] bn::array<bn::sprite_tiles_ptr, player_health_tile_data.size()> make_player_health_tiles()
    {
        return {{ player_health_tiles_item.create_tiles(0), player_health_tiles_item.create_tiles(1) }};
    }

    [[nodiscard]] constexpr int stage_glyph_index(char character)
    {
        switch(character)
        {
        case 'A': return 0;
        case 'D': return 1;
        case 'E': return 2;
        case 'G': return 3;
        case 'I': return 4;
        case 'O': return 5;
        case 'R': return 6;
        case 'T': return 7;
        case 'X': return 8;
        case 'Y': return 9;
        case '!': return 10;
        case 'S': return 11;
        case '1': return 12;
        case '2': return 13;
        case '3': return 14;
        case '4': return 15;
        case '5': return 16;
        case 'C': return 17;
        case 'L': return 18;
        case 'M': return 19;
        case 'N': return 20;
        case 'U': return 21;
        case 'V': return 22;
        default: return -1;
        }
    }

    [[nodiscard]] constexpr int stage_number(StageId stage)
    {
        return int(stage) + 1;
    }

    [[nodiscard]] constexpr bool phase_runs_gameplay(GameScene::StagePhase phase)
    {
        return phase == GameScene::StagePhase::PLAYING;
    }

    [[nodiscard]] constexpr bool phase_allows_player_movement(GameScene::StagePhase phase)
    {
        return phase == GameScene::StagePhase::PLAYING || phase == GameScene::StagePhase::CLEARED;
    }

    [[nodiscard]] constexpr GameScene::StagePhase stage_phase_after_ticks(int frames)
    {
        GameScene::StagePhase phase = GameScene::StagePhase::INTRO;
        int frames_remaining = intro_frames;

        for(int index = 0; index < frames; ++index)
        {
            --frames_remaining;
            if(frames_remaining > 0)
            {
                continue;
            }

            if(phase == GameScene::StagePhase::INTRO)
            {
                phase = GameScene::StagePhase::READY;
                frames_remaining = ready_frames;
            }
            else if(phase == GameScene::StagePhase::READY)
            {
                phase = GameScene::StagePhase::GO;
                frames_remaining = go_frames;
            }
            else if(phase == GameScene::StagePhase::GO)
            {
                phase = GameScene::StagePhase::PLAYING;
            }
        }

        return phase;
    }

    static_assert(player_bounds.min_x == -72 && player_bounds.max_x == 72);
    static_assert(player_bounds.min_y == -72 && player_bounds.max_y == 72);
    static_assert(GameScene::enemy_count + 1 == SpatialManager::actor_count);
    static_assert(! stage_enemy_respawn_enabled);
    static_assert(2 + max_hitboxes_per_frame + 2 + CrossbowProjectilePool::capacity <=
                  CollisionDebugBoxList::capacity);
    static_assert(player_damage_applies_in_debug(false, 1));
    static_assert(! player_damage_applies_in_debug(true, 1));
    static_assert(! player_damage_applies_in_debug(false, 0));
    static_assert(cycled_debug_enemy_type_index(0, 2, 1) == 1);
    static_assert(cycled_debug_enemy_type_index(0, 2, -1) == 1);
    static_assert(4 + 2 + CrossbowProjectilePool::capacity == CollisionDebugBoxList::capacity);
    static_assert(stage_phase_after_ticks(intro_frames - 1) ==
                  GameScene::StagePhase::INTRO);
    static_assert(stage_phase_after_ticks(intro_frames) ==
                  GameScene::StagePhase::READY);
    static_assert(stage_phase_after_ticks(intro_frames + ready_frames - 1) ==
                  GameScene::StagePhase::READY);
    static_assert(stage_phase_after_ticks(intro_frames + ready_frames) ==
                  GameScene::StagePhase::GO);
    static_assert(stage_phase_after_ticks(intro_frames + ready_frames + go_frames - 1) ==
                  GameScene::StagePhase::GO);
    static_assert(stage_phase_after_ticks(intro_frames + ready_frames + go_frames) ==
                  GameScene::StagePhase::PLAYING);
    static_assert(! phase_runs_gameplay(GameScene::StagePhase::INTRO));
    static_assert(! phase_runs_gameplay(GameScene::StagePhase::READY));
    static_assert(! phase_runs_gameplay(GameScene::StagePhase::GO));
    static_assert(phase_runs_gameplay(GameScene::StagePhase::PLAYING));
    static_assert(phase_allows_player_movement(GameScene::StagePhase::CLEARED));
    static_assert(int(StageId::STAGE_1) == 0);
    static_assert(int(StageId::STAGE_2) == 1);
    static_assert(int(StageId::STAGE_3) == 2);
    static_assert(int(StageId::STAGE_4) == 3);
    static_assert(int(StageId::STAGE_5) == 4);
    static_assert(stage_number(StageId::STAGE_1) == 1);
    static_assert(stage_number(StageId::STAGE_2) == 2);
    static_assert(stage_number(StageId::STAGE_3) == 3);
    static_assert(stage_number(StageId::STAGE_4) == 4);
    static_assert(stage_number(StageId::STAGE_5) == 5);
    static_assert(stage_glyph_index('3') >= 0);
    static_assert(stage_glyph_index('4') >= 0);
    static_assert(stage_glyph_index('5') >= 0);
    static_assert(stage_glyph_index('V') >= 0);
}

GameScene::GameScene() :
    _player(stage_definition(StageId::STAGE_1).player_spawn),
    _player_bounds(player_bounds),
    _enemy_runtime(),
    _stage_glyph_tiles(make_stage_glyph_tiles()),
    // This shares the title glyph palette and keeps it alive after TitleScene::hide().
    _stage_glyph_palette(stage_glyph_item.palette_item().create_palette()),
    _player_health_tiles(make_player_health_tiles()),
    _spatial_debug_overlay(stage_definition(StageId::STAGE_1).ground_stage),
    _spatial_manager(stage_definition(StageId::STAGE_1).ground_stage,
                     stage_definition(StageId::STAGE_1).ground_static_obstacles,
                     stage_definition(StageId::STAGE_1).upper_stage,
                     stage_definition(StageId::STAGE_1).upper_static_obstacles)
{
    int health_x = player_health_hud_x;
    const CharacterDefinition& player_definition = _player.definition();
    for(int index = 0; index < player_definition.display_name_length; ++index)
    {
        int glyph_index = stage_glyph_index(player_definition.display_name[index]);
        BN_ASSERT(glyph_index >= 0, "Unsupported HUD glyph");

        bn::sprite_builder sprite_builder(
                stage_glyph_item.shape_size(), _stage_glyph_tiles[glyph_index], _stage_glyph_palette);
        sprite_builder.set_position(health_x, player_health_hud_y);
        sprite_builder.set_z_order(-4);
        bn::sprite_ptr sprite = sprite_builder.release_build();
        sprite.set_visible(false);
        _player_name_sprites.push_back(bn::move(sprite));
        health_x += player_name_hud_spacing;
    }

    health_x += player_name_to_health_spacing;
    for(int index = 0; index < _player.max_health(); ++index)
    {
        bn::sprite_builder sprite_builder(
                bn::sprite_shape_size(8, 8), _player_health_tiles[1], _stage_glyph_palette);
        sprite_builder.set_position(health_x + index * player_health_hud_spacing, player_health_hud_y);
        sprite_builder.set_z_order(-4);
        bn::sprite_ptr sprite = sprite_builder.release_build();
        sprite.set_visible(false);
        _player_health_sprites.push_back(bn::move(sprite));
    }

    _player.set_visible(false);
    for(Goblin& goblin : _goblins)
    {
        goblin.hide();
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.hide();
    }
}

void GameScene::enter()
{
    bn::bg_palettes::set_transparent_color(game_background_color);
    _battlefield.set_visible(true);
    _player.reset_health();
    _update_player_health_hud();
    _set_player_hud_visible(true);
    _start_stage(StageId::STAGE_1);
}

void GameScene::exit()
{
    _battlefield.set_visible(false);
    _player.set_visible(false);
    _player.melee_attack().clear_visual_effect();
    for(Goblin& goblin : _goblins)
    {
        goblin.hide();
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.hide();
    }
    _crossbow_projectiles.clear();
    _hit_effects.clear();
    _collision_debug_overlay.reset();
    _spatial_debug_overlay.reset();
    _set_player_hud_visible(false);
    _clear_stage_message();
}

bool GameScene::update()
{
#if defined(GAIN_DEBUG_LOGS) || defined(GAIN_PERF_DEBUG_LOGS)
    ++_debug_log_frame_count;

    if(_debug_log_frame_count % 60 == 0)
    {
        int current_stage_number = stage_number(_stage);

    #if defined(GAIN_DEBUG_LOGS)
        BN_LOG_LEVEL(bn::log_level::DEBUG, "[DEBUG] stage=", current_stage_number,
                     " frame=", _debug_log_frame_count);
    #endif

    #if defined(GAIN_PERF_DEBUG_LOGS)
        BN_LOG_LEVEL(bn::log_level::INFO, "[PERF] stage=", current_stage_number,
                     " frame=", _debug_log_frame_count);
    #endif
    }
#endif

    if(_stage_phase == StagePhase::PLAYER_DEAD)
    {
        if(--_phase_frames_remaining == 0)
        {
            _clear_stage_message();
            return true;
        }

        return false;
    }

    if(_stage_phase == StagePhase::CONGRATULATIONS || _stage_phase == StagePhase::GAME_OVER)
    {
        if(--_phase_frames_remaining == 0)
        {
            if(_stage_phase == StagePhase::CONGRATULATIONS)
            {
                _stage_phase = StagePhase::GAME_OVER;
                _phase_frames_remaining = game_over_frames;
                _set_stage_message("GAME OVER", 9, stage_message_y, 12, 1);
            }
            else
            {
                _clear_stage_message();
                return true;
            }
        }
        return false;
    }

    _update_stage_phase();

    if(_stage_phase == StagePhase::PLAYING)
    {
        _update_playing();
    }
    else if(_stage_phase == StagePhase::CLEARED)
    {
        _update_cleared();
    }

    if(bn::keypad::select_pressed())
    {
        _collision_debug_overlay.toggle();
        _spatial_debug_overlay.set_visible(_collision_debug_overlay.enabled());
        if(_collision_debug_overlay.enabled())
        {
            _validate_debug_enemy_type();
        }
    }

    if(_collision_debug_overlay.enabled())
    {
        if(bn::keypad::l_pressed())
        {
            _cycle_debug_enemy_type(1);
        }
        else if(bn::keypad::r_pressed())
        {
            _cycle_debug_enemy_representative();
        }
    }

    _update_collision_debug_overlay();
    return false;
}

void GameScene::_start_stage(StageId stage)
{
    _stage = stage;

#if defined(GAIN_DEBUG_LOGS) || defined(GAIN_PERF_DEBUG_LOGS)
    int current_stage_number = stage_number(_stage);
#endif

#if defined(GAIN_DEBUG_LOGS)
    BN_LOG_LEVEL(bn::log_level::DEBUG, "[DEBUG] stage=", current_stage_number, " entered");
#endif

#if defined(GAIN_PERF_DEBUG_LOGS)
    BN_LOG_LEVEL(bn::log_level::INFO, "[PERF] stage=", current_stage_number, " entered");
#endif

    _stage_phase = StagePhase::INTRO;
    _phase_frames_remaining = intro_frames;
    _enemy_runtime.clear_roster();
    const StageDefinition& definition = stage_definition(_stage);
    _battlefield.set_stage(definition.visual);
    _spatial_manager.set_stage(definition.ground_stage, definition.ground_static_obstacles,
                               definition.upper_stage, definition.upper_static_obstacles);
    _spatial_debug_overlay.set_stage(definition.ground_stage);
    _player.apply_movement(definition.player_spawn, Direction::DOWN);
    _player.set_spatial_layer(definition.player_layer);
    _player.set_visible(true);

    for(Goblin& goblin : _goblins)
    {
        goblin.deactivate();
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.deactivate();
    }

    for(int index = 0; index < definition.enemy_spawn_count; ++index)
    {
        const StageEnemySpawn& spawn = definition.enemy_spawns[index];
        ActiveEnemy& slot = _enemy_runtime.allocate_enemy(spawn.type);
        switch(spawn.type)
        {
        case EnemyType::GOBLIN:
        {
            Goblin& goblin = _goblins[slot.pool_index];
            goblin.set_home_position(spawn.position);
            goblin.set_spatial_layer(spawn.layer);
            goblin.set_respawn_enabled(stage_enemy_respawn_enabled);
            goblin.enter();
            break;
        }
        case EnemyType::CROSSBOW:
        {
            CrossbowGoblin& crossbow_goblin = _crossbow_goblins[slot.pool_index];
            crossbow_goblin.set_home_position(spawn.position);
            crossbow_goblin.set_spatial_layer(spawn.layer);
            crossbow_goblin.set_respawn_enabled(stage_enemy_respawn_enabled);
            crossbow_goblin.enter();
            break;
        }
        case EnemyType::NONE:
            BN_ASSERT(false, "Stage enemy spawn has no type");
            break;
        default:
            BN_ASSERT(false, "Unknown stage enemy type");
            break;
        }
    }

    _player.melee_attack().clear_visual_effect();
    _crossbow_projectiles.clear();
    _hit_effects.clear();
    _collision_debug_overlay.clear();
    _spatial_debug_overlay.set_visible(_collision_debug_overlay.enabled());
    _validate_debug_enemy_type();
    _sync_spatial_actors();
    char stage_text[] = "STAGE 0";
    stage_text[6] = char('0' + stage_number(_stage));
    _set_stage_message(stage_text, 7, stage_message_y, 16, 2);
}

void GameScene::_update_stage_phase()
{
    if(_stage_phase != StagePhase::INTRO && _stage_phase != StagePhase::READY && _stage_phase != StagePhase::GO)
    {
        return;
    }

    --_phase_frames_remaining;
    if(_phase_frames_remaining > 0)
    {
        return;
    }

    if(_stage_phase == StagePhase::INTRO)
    {
        _stage_phase = StagePhase::READY;
        _phase_frames_remaining = ready_frames;
        _set_stage_message("READY!", 6, stage_message_y, 16, 2);
    }
    else if(_stage_phase == StagePhase::READY)
    {
        _stage_phase = StagePhase::GO;
        _phase_frames_remaining = go_frames;
        _set_stage_message("GO!", 3, stage_message_y, 16, 2);
    }
    else
    {
        _stage_phase = StagePhase::PLAYING;
        _clear_stage_message();
    }
}

void GameScene::_update_playing()
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
                        world_box(_player.position(), _player.collision_body().pushbox.box),
                        _player.spatial_layer(), true);

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();

    if(! _stage_has_enemies())
    {
        _hit_effects.update();
        return;
    }

    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    bn::fixed_point player_foot_position = _player.foot_position();
    for(int roster_index = 0; roster_index < EnemyRuntime::active_enemy_capacity; ++roster_index)
    {
        const ActiveEnemy& slot = _enemy_runtime.active_enemy(roster_index);
        if(! slot.occupied)
        {
            continue;
        }

        switch(slot.type)
        {
        case EnemyType::GOBLIN:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::goblin_count);
            Goblin& goblin = _goblins[slot.pool_index];
            goblin.update(player_foot_position,
                          goblin.spatial_layer() == _player.spatial_layer(), _spatial_manager.movement_obstacles(
                                  slot.actor_id, _movement_query_area(goblin.movement_obstacle_query_area())));
            _sync_spatial_actor(slot.actor_id, goblin.world_pushbox(),
                                goblin.spatial_layer(), goblin.active());
            break;
        }
        case EnemyType::CROSSBOW:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::crossbow_goblin_count);
            CrossbowGoblin& crossbow_goblin = _crossbow_goblins[slot.pool_index];
            crossbow_goblin.update(player_hurtbox, player_foot_position, _spatial_manager.movement_obstacles(
                    slot.actor_id, _movement_query_area(crossbow_goblin.movement_obstacle_query_area())),
                    _crossbow_projectiles);
            _sync_spatial_actor(slot.actor_id, crossbow_goblin.world_pushbox(),
                                crossbow_goblin.spatial_layer(), crossbow_goblin.active());
            break;
        }
        case EnemyType::NONE:
            break;
        default:
            BN_ASSERT(false, "Unknown enemy type");
            break;
        }
    }
    _crossbow_projectiles.update();

    int player_damage = 0;
    for(int roster_index = 0; roster_index < EnemyRuntime::active_enemy_capacity; ++roster_index)
    {
        const ActiveEnemy& slot = _enemy_runtime.active_enemy(roster_index);
        if(! slot.occupied)
        {
            continue;
        }

        switch(slot.type)
        {
        case EnemyType::GOBLIN:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::goblin_count);
            Goblin& goblin = _goblins[slot.pool_index];
            const bool active_before_combat = goblin.active();
            goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
            player_damage += goblin.resolve_player_hit(
                    _player.position(), _player.collision_body().hurtbox, _hit_effects);
            if(goblin.active() != active_before_combat)
            {
                _sync_spatial_actor(slot.actor_id, goblin.world_pushbox(),
                                    goblin.spatial_layer(), goblin.active());
            }
            break;
        }
        case EnemyType::CROSSBOW:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::crossbow_goblin_count);
            CrossbowGoblin& crossbow_goblin = _crossbow_goblins[slot.pool_index];
            const bool active_before_combat = crossbow_goblin.active();
            crossbow_goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
            if(crossbow_goblin.active() != active_before_combat)
            {
                _sync_spatial_actor(slot.actor_id, crossbow_goblin.world_pushbox(),
                                    crossbow_goblin.spatial_layer(), crossbow_goblin.active());
            }
            break;
        }
        case EnemyType::NONE:
            break;
        default:
            BN_ASSERT(false, "Unknown enemy type");
            break;
        }
    }
    player_damage += _crossbow_projectiles.resolve_player_hit(
            _player.position(), _player.collision_body().hurtbox, _hit_effects);
#if defined(GAIN_DEBUG_LOGS)
    _spatial_manager.validate_position_table();
#endif
    _hit_effects.update();

    _apply_player_damage(player_damage);
    if(_player.dead())
    {
        return;
    }

    if(_all_stage_enemies_defeated())
    {
        _stage_phase = StagePhase::CLEARED;
        _set_stage_message("EXIT", 4, exit_message_y, 8, 1);
    }
}

void GameScene::_update_cleared()
{
    _update_player_gameplay();
    _crossbow_projectiles.update();
    _hit_effects.update();

    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    const StageDefinition& definition = stage_definition(_stage);
    const WorldBox& exit_box = definition.exit_box;
    if(touches_or_intersects(player_pushbox, exit_box))
    {
        if(_stage == StageId::STAGE_1)
        {
            _start_stage(StageId::STAGE_2);
        }
        else if(_stage == StageId::STAGE_2)
        {
            _start_stage(StageId::STAGE_3);
        }
        else
        {
            _stage_phase = StagePhase::CONGRATULATIONS;
            _phase_frames_remaining = congratulations_frames;
            _set_stage_message("CONGRATULATIONS!", 16, stage_message_y, 8, 1);
        }
    }
}

void GameScene::_update_player_gameplay()
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
                        world_box(_player.position(), _player.collision_body().pushbox.box),
                        _player.spatial_layer(), true);

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
}

void GameScene::_apply_player_damage(int damage)
{
    if(! player_damage_applies_in_debug(_collision_debug_overlay.enabled(), damage))
    {
        return;
    }

    _player.take_damage(damage);
    _update_player_health_hud();
    if(_player.dead())
    {
        _stage_phase = StagePhase::PLAYER_DEAD;
        _phase_frames_remaining = player_dead_frames;
        _set_stage_message("YOU DIED", 8, stage_message_y, 12, 1);
    }
}

void GameScene::_update_player_health_hud()
{
    for(int index = 0; index < _player_health_sprites.size(); ++index)
    {
        _player_health_sprites[index].set_tiles(_player_health_tiles[index < _player.current_health() ? 1 : 0]);
    }
}

void GameScene::_set_player_hud_visible(bool visible)
{
    for(bn::sprite_ptr& sprite : _player_name_sprites)
    {
        sprite.set_visible(visible);
    }

    for(bn::sprite_ptr& sprite : _player_health_sprites)
    {
        sprite.set_visible(visible);
    }
}

void GameScene::_set_stage_message(const char* text, int character_count, int y, int spacing, bn::fixed scale)
{
    _clear_stage_message();

    int x = -((character_count - 1) * spacing) / 2;
    for(int index = 0; index < character_count; ++index)
    {
        int glyph_index = stage_glyph_index(text[index]);
        if(glyph_index >= 0)
        {
            bn::sprite_builder sprite_builder(
                    stage_glyph_item.shape_size(), _stage_glyph_tiles[glyph_index], _stage_glyph_palette);
            sprite_builder.set_position(x, y);
            sprite_builder.set_scale(scale);
            sprite_builder.set_z_order(-4);
            bn::sprite_ptr sprite = sprite_builder.release_build();
            _stage_message_sprites.push_back(bn::move(sprite));
        }

        x += spacing;
    }
}

void GameScene::_clear_stage_message()
{
    _stage_message_sprites.clear();
}

bool GameScene::_stage_has_enemies() const
{
    return true;
}

bool GameScene::_all_stage_enemies_defeated() const
{
    for(int roster_index = 0; roster_index < EnemyRuntime::active_enemy_capacity; ++roster_index)
    {
        const ActiveEnemy& slot = _enemy_runtime.active_enemy(roster_index);
        if(! slot.occupied)
        {
            continue;
        }

        switch(slot.type)
        {
        case EnemyType::GOBLIN:
            BN_ASSERT(slot.pool_index < EnemyRuntime::goblin_count);
            if(_goblins[slot.pool_index].active())
            {
                return false;
            }
            break;
        case EnemyType::CROSSBOW:
            BN_ASSERT(slot.pool_index < EnemyRuntime::crossbow_goblin_count);
            if(_crossbow_goblins[slot.pool_index].active())
            {
                return false;
            }
            break;
        case EnemyType::NONE:
            break;
        default:
            BN_ASSERT(false, "Unknown enemy type");
            break;
        }
    }

    return true;
}

void GameScene::_update_collision_debug_overlay()
{
    if(! _collision_debug_overlay.enabled())
    {
        return;
    }

    CollisionDebugBoxList boxes;
    CollisionDebugRadiusList radii;
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    boxes.add(player_hurtbox, CollisionDebugBoxType::HURTBOX);
    boxes.add(world_box(_player.position(), _player.collision_body().pushbox.box), CollisionDebugBoxType::PUSHBOX);
    WorldBoxList<max_hitboxes_per_frame> player_hitboxes = _player.melee_attack().active_hitboxes();
    for(int index = 0; index < player_hitboxes.count; ++index)
    {
        boxes.add(player_hitboxes.boxes[index], CollisionDebugBoxType::HITBOX);
    }
    if(Enemy* debug_enemy = _debug_enemy_representative())
    {
        debug_enemy->append_debug_shapes(boxes, radii);
        _crossbow_projectiles.append_collision_debug_boxes(debug_enemy->actor_id(), boxes);
    }
    _collision_debug_overlay.update(boxes, radii);
}

bn::vector<CharacterId, GameScene::enemy_count> GameScene::_active_debug_enemy_types() const
{
    bn::vector<CharacterId, enemy_count> result;
    for(const Goblin& goblin : _goblins)
    {
        append_debug_enemy_type(goblin, result);
    }
    for(const CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        append_debug_enemy_type(crossbow_goblin, result);
    }
    return result;
}

void GameScene::_validate_debug_enemy_type()
{
    bn::vector<CharacterId, enemy_count> types = _active_debug_enemy_types();
    if(types.empty())
    {
        _debug_enemy_type.reset();
        _debug_enemy_actor_id.reset();
        return;
    }

    if(_debug_enemy_type)
    {
        for(CharacterId type : types)
        {
            if(type == *_debug_enemy_type)
            {
                return;
            }
        }

        // The selected type was eliminated. Continue the same type-cycle
        // order when another active type follows it, then wrap to the first.
        for(CharacterId type : types)
        {
            if(int(type) > int(*_debug_enemy_type))
            {
                _debug_enemy_type = type;
                _debug_enemy_actor_id.reset();
                return;
            }
        }
    }

    _debug_enemy_type = types.front();
    _debug_enemy_actor_id.reset();
}

void GameScene::_cycle_debug_enemy_type(int offset)
{
    _validate_debug_enemy_type();
    if(! _debug_enemy_type)
    {
        return;
    }

    bn::vector<CharacterId, enemy_count> types = _active_debug_enemy_types();
    for(int index = 0; index < types.size(); ++index)
    {
        if(types[index] == *_debug_enemy_type)
        {
            int next_index = cycled_debug_enemy_type_index(index, types.size(), offset);
            _debug_enemy_type = types[next_index];
            _debug_enemy_actor_id.reset();
            return;
        }
    }
}

void GameScene::_cycle_debug_enemy_representative()
{
    _validate_debug_enemy_type();
    if(! _debug_enemy_type)
    {
        return;
    }

    Enemy* first = nullptr;
    bool selected_found = false;
    auto select_next = [&](Enemy& enemy) -> bool
    {
        if(! enemy.active() || enemy.definition().id != *_debug_enemy_type)
        {
            return false;
        }

        if(! first)
        {
            first = &enemy;
        }
        if(selected_found)
        {
            _debug_enemy_actor_id = enemy.actor_id();
            return true;
        }
        if(_debug_enemy_actor_id && enemy.actor_id() == *_debug_enemy_actor_id)
        {
            selected_found = true;
        }
        return false;
    };

    for(Goblin& goblin : _goblins)
    {
        if(select_next(goblin))
        {
            return;
        }
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        if(select_next(crossbow_goblin))
        {
            return;
        }
    }

    if(first)
    {
        _debug_enemy_actor_id = first->actor_id();
    }
}

Enemy* GameScene::_first_debug_enemy_of_type(CharacterId type)
{
    for(Goblin& goblin : _goblins)
    {
        if(goblin.active() && goblin.definition().id == type)
        {
            _debug_enemy_actor_id = goblin.actor_id();
            return &goblin;
        }
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        if(crossbow_goblin.active() && crossbow_goblin.definition().id == type)
        {
            _debug_enemy_actor_id = crossbow_goblin.actor_id();
            return &crossbow_goblin;
        }
    }
    return nullptr;
}

Enemy* GameScene::_debug_enemy_representative()
{
    _validate_debug_enemy_type();
    if(! _debug_enemy_type)
    {
        return nullptr;
    }

    for(Goblin& goblin : _goblins)
    {
        if(goblin.active() && goblin.definition().id == *_debug_enemy_type && _debug_enemy_actor_id &&
           goblin.actor_id() == *_debug_enemy_actor_id)
        {
            return &goblin;
        }
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        if(crossbow_goblin.active() && crossbow_goblin.definition().id == *_debug_enemy_type &&
           _debug_enemy_actor_id && crossbow_goblin.actor_id() == *_debug_enemy_actor_id)
        {
            return &crossbow_goblin;
        }
    }

    return _first_debug_enemy_of_type(*_debug_enemy_type);
}

void GameScene::_sync_spatial_actors()
{
    _sync_spatial_actor(SpatialActorId::PLAYER,
                        world_box(_player.position(), _player.collision_body().pushbox.box),
                        _player.spatial_layer(), true);
    for(int roster_index = 0; roster_index < EnemyRuntime::active_enemy_capacity; ++roster_index)
    {
        const ActiveEnemy& slot = _enemy_runtime.active_enemy(roster_index);
        if(! slot.occupied)
        {
            continue;
        }

        switch(slot.type)
        {
        case EnemyType::GOBLIN:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::goblin_count);
            const Goblin& goblin = _goblins[slot.pool_index];
            _sync_spatial_actor(slot.actor_id, goblin.world_pushbox(), goblin.spatial_layer(), goblin.active());
            break;
        }
        case EnemyType::CROSSBOW:
        {
            BN_ASSERT(slot.pool_index < EnemyRuntime::crossbow_goblin_count);
            const CrossbowGoblin& crossbow_goblin = _crossbow_goblins[slot.pool_index];
            _sync_spatial_actor(slot.actor_id, crossbow_goblin.world_pushbox(),
                                crossbow_goblin.spatial_layer(), crossbow_goblin.active());
            break;
        }
        case EnemyType::NONE:
            break;
        default:
            BN_ASSERT(false, "Unknown enemy type");
            break;
        }
    }

#if defined(GAIN_DEBUG_LOGS)
    _spatial_manager.validate_position_table();
#endif
}

void GameScene::_sync_spatial_actor(SpatialActorId actor_id, const WorldBox& pushbox,
                                    SpatialLayer layer, bool active)
{
    _spatial_manager.update_actor(actor_id, pushbox, layer);
    _spatial_manager.set_actor_active(actor_id, active);
}

WorldBox GameScene::_movement_query_area(const WorldBox& pushbox)
{
    return { pushbox.center, pushbox.width + movement_query_padding * 2,
             pushbox.height + movement_query_padding * 2 };
}

#undef _goblins
#undef _crossbow_goblins
