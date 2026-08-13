#include "scene/game_scene.h"

#include "bn_bg_palettes.h"
#include "bn_array.h"
#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_sprite_item.h"
#include "bn_tile.h"

#include "combat/collision/movement_collision.h"
#include "combat/collision/collision_math.h"
#include "world/stages/stage1.h"
#include "world/stages/stage2.h"

namespace
{
    constexpr bn::color game_background_color(3, 12, 7);
    constexpr int player_size = 16;
    constexpr int player_start_x = 0;
    constexpr int player_start_y = 56;
    constexpr bn::array<bn::fixed_point, GameScene::goblin_count> goblin_home_positions = {{
        { -48, -40 }, { 48, -40 }, { -40, -4 }, { 40, -4 }
    }};
    constexpr bn::array<int, GameScene::goblin_count> goblin_target_ids = {{ 10, 11, 12, 13 }};
    constexpr bn::array<bn::fixed_point, GameScene::crossbow_goblin_count> crossbow_goblin_home_positions = {{
        { 0, -48 }, { 0, -48 }, { 0, -48 }, { 0, -48 }
    }};
    constexpr bn::array<int, GameScene::crossbow_goblin_count> crossbow_goblin_target_ids = {{ 14, 15, 16, 17 }};
    constexpr int stage1_active_goblin_count = 4;
    constexpr int stage1_active_crossbow_goblin_count = 1;
    constexpr int stage2_active_goblin_count = 2;
    constexpr int stage2_active_crossbow_goblin_count = 4;
    constexpr bool stage_enemy_respawn_enabled = false;
    constexpr int movement_query_padding = 1;
    constexpr int intro_frames = 90;
    constexpr int ready_frames = 120;
    constexpr int go_frames = 45;
    constexpr int congratulations_frames = 120;
    constexpr int game_over_frames = 120;
    constexpr int stage_message_y = 0;
    constexpr int exit_message_y = -65;
    constexpr bn::array<SpatialActorId, GameScene::goblin_count> goblin_spatial_actor_ids = {{
        SpatialActorId::GOBLIN_0, SpatialActorId::GOBLIN_1,
        SpatialActorId::GOBLIN_2, SpatialActorId::GOBLIN_3
    }};
    constexpr bn::array<SpatialActorId, GameScene::crossbow_goblin_count> crossbow_spatial_actor_ids = {{
        SpatialActorId::CROSSBOW_GOBLIN_0, SpatialActorId::CROSSBOW_GOBLIN_1,
        SpatialActorId::CROSSBOW_GOBLIN_2, SpatialActorId::CROSSBOW_GOBLIN_3
    }};

    constexpr MovementBounds player_bounds = Battlefield::movement_bounds(player_size, player_size);

    [[nodiscard]] constexpr bool stage1_spawn_cell_is_walkable(const bn::fixed_point& position)
    {
        int cell_x = stage_cell_from_world_coordinate(stage1::data, int(position.x()));
        int cell_y = stage_cell_from_world_coordinate(stage1::data, int(position.y()));
        return stage_cell_in_bounds(stage1::data, cell_x, cell_y) &&
               stage_cell_at(stage1::data, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr bool stage1_spawns_are_separated()
    {
        constexpr WorldBox player_pushbox = { { player_start_x, player_start_y + 1 }, 8, 8 };
        constexpr WorldBox crossbow_pushbox = { { 0, -45 }, 6, 6 };
        constexpr WorldBox goblin_pushboxes[GameScene::goblin_count] = {
            { { -48, -37 }, 6, 6 }, { { 48, -37 }, 6, 6 },
            { { -40, -1 }, 6, 6 }, { { 40, -1 }, 6, 6 }
        };

        if(overlaps_strictly(player_pushbox, crossbow_pushbox) ||
           overlaps_strictly(player_pushbox, stage1::static_obstacle_boxes[0]) ||
           overlaps_strictly(crossbow_pushbox, stage1::static_obstacle_boxes[0]))
        {
            return false;
        }

        for(int first = 0; first < GameScene::goblin_count; ++first)
        {
            if(overlaps_strictly(player_pushbox, goblin_pushboxes[first]) ||
               overlaps_strictly(crossbow_pushbox, goblin_pushboxes[first]) ||
               overlaps_strictly(goblin_pushboxes[first], stage1::static_obstacle_boxes[0]))
            {
                return false;
            }

            for(int second = first + 1; second < GameScene::goblin_count; ++second)
            {
                if(overlaps_strictly(goblin_pushboxes[first], goblin_pushboxes[second]))
                {
                    return false;
                }
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool stage2_spawn_cell_is_walkable(
            const StageData& stage, const bn::fixed_point& position)
    {
        int cell_x = stage_cell_from_world_coordinate(stage, int(position.x()));
        int cell_y = stage_cell_from_world_coordinate(stage, int(position.y()));
        return stage_cell_in_bounds(stage, cell_x, cell_y) &&
               stage_cell_at(stage, cell_x, cell_y) == StageCell::WALKABLE;
    }

    [[nodiscard]] constexpr bool stage2_same_layer_spawns_are_separated()
    {
        constexpr WorldBox player_pushbox = { { stage2::player_spawn.x(), stage2::player_spawn.y() + 1 }, 8, 8 };
        constexpr WorldBox ground_goblin_pushboxes[2] = {
            { { stage2::goblin_spawns[0].x(), stage2::goblin_spawns[0].y() + 3 }, 6, 6 },
            { { stage2::goblin_spawns[1].x(), stage2::goblin_spawns[1].y() + 3 }, 6, 6 }
        };
        constexpr WorldBox upper_crossbow_pushboxes[GameScene::crossbow_goblin_count] = {
            { { stage2::crossbow_spawns[0].x(), stage2::crossbow_spawns[0].y() + 3 }, 6, 6 },
            { { stage2::crossbow_spawns[1].x(), stage2::crossbow_spawns[1].y() + 3 }, 6, 6 },
            { { stage2::crossbow_spawns[2].x(), stage2::crossbow_spawns[2].y() + 3 }, 6, 6 },
            { { stage2::crossbow_spawns[3].x(), stage2::crossbow_spawns[3].y() + 3 }, 6, 6 }
        };

        return ! overlaps_strictly(player_pushbox, ground_goblin_pushboxes[0]) &&
               ! overlaps_strictly(player_pushbox, ground_goblin_pushboxes[1]) &&
               ! overlaps_strictly(ground_goblin_pushboxes[0], ground_goblin_pushboxes[1]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[0], upper_crossbow_pushboxes[1]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[0], upper_crossbow_pushboxes[2]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[0], upper_crossbow_pushboxes[3]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[1], upper_crossbow_pushboxes[2]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[1], upper_crossbow_pushboxes[3]) &&
               ! overlaps_strictly(upper_crossbow_pushboxes[2], upper_crossbow_pushboxes[3]);
    }

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

    constexpr bn::array<bn::tile, 20> stage_glyph_tiles = {
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
        make_stage_glyph({ 15, 16, 16, 16, 16, 16, 15 }),  // C
        make_stage_glyph({ 16, 16, 16, 16, 16, 16, 31 }),  // L
        make_stage_glyph({ 17, 27, 21, 17, 17, 17, 17 }),  // M
        make_stage_glyph({ 17, 25, 21, 19, 17, 17, 17 }),  // N
        make_stage_glyph({ 17, 17, 17, 17, 17, 17, 14 }),  // U
        make_stage_glyph({ 17, 17, 17, 10, 10, 4, 4 })     // V
    };

    constexpr bn::array<bn::color, 16> stage_glyph_colors = {
        bn::color(0, 0, 0), bn::color(31, 28, 16), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color(),
        bn::color(), bn::color(), bn::color(), bn::color()
    };

    constexpr bn::sprite_item stage_glyph_item(
            bn::sprite_shape_size(8, 8), stage_glyph_tiles, stage_glyph_colors,
            bn::bpp_mode::BPP_4, stage_glyph_tiles.size());

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
        case 'C': return 14;
        case 'L': return 15;
        case 'M': return 16;
        case 'N': return 17;
        case 'U': return 18;
        case 'V': return 19;
        default: return -1;
        }
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

    [[nodiscard]] constexpr bool crossbow_target_ids_are_unique()
    {
        for(int first = 0; first < GameScene::crossbow_goblin_count; ++first)
        {
            for(int second = first + 1; second < GameScene::crossbow_goblin_count; ++second)
            {
                if(crossbow_goblin_target_ids[first] == crossbow_goblin_target_ids[second])
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
    static_assert(crossbow_target_ids_are_unique());
    static_assert(stage1_active_goblin_count == GameScene::goblin_count);
    static_assert(stage1_active_crossbow_goblin_count == 1);
    static_assert(stage2_active_goblin_count == 2);
    static_assert(stage2_active_crossbow_goblin_count == GameScene::crossbow_goblin_count);
    static_assert(GameScene::enemy_count + 1 == SpatialManager::actor_count);
    static_assert(stage1_spawn_cell_is_walkable({ player_start_x, player_start_y }));
    static_assert(stage1_spawn_cell_is_walkable(crossbow_goblin_home_positions[0]));
    static_assert(stage1_spawn_cell_is_walkable(goblin_home_positions[0]));
    static_assert(stage1_spawn_cell_is_walkable(goblin_home_positions[1]));
    static_assert(stage1_spawn_cell_is_walkable(goblin_home_positions[2]));
    static_assert(stage1_spawn_cell_is_walkable(goblin_home_positions[3]));
    static_assert(stage1_spawns_are_separated());
    static_assert(stage2_spawn_cell_is_walkable(stage2::ground_data, stage2::player_spawn));
    static_assert(stage2_spawn_cell_is_walkable(stage2::ground_data, stage2::goblin_spawns[0]));
    static_assert(stage2_spawn_cell_is_walkable(stage2::ground_data, stage2::goblin_spawns[1]));
    static_assert(stage2_spawn_cell_is_walkable(stage2::upper_data, stage2::crossbow_spawns[0]));
    static_assert(stage2_spawn_cell_is_walkable(stage2::upper_data, stage2::crossbow_spawns[1]));
    static_assert(stage2_spawn_cell_is_walkable(stage2::upper_data, stage2::crossbow_spawns[2]));
    static_assert(stage2_spawn_cell_is_walkable(stage2::upper_data, stage2::crossbow_spawns[3]));
    static_assert(stage2_same_layer_spawns_are_separated());
    static_assert(! stage_enemy_respawn_enabled);
    static_assert(2 + max_hitboxes_per_frame + (GameScene::goblin_count * 3) + 8 +
                  CrossbowProjectilePool::capacity + stage1::static_obstacle_count ==
                  CollisionDebugBoxList::capacity);
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
    static_assert(stage_glyph_index('V') >= 0);
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
    _crossbow_goblins{{
        CrossbowGoblin(crossbow_goblin_home_positions[0], crossbow_goblin_target_ids[0]),
        CrossbowGoblin(crossbow_goblin_home_positions[1], crossbow_goblin_target_ids[1]),
        CrossbowGoblin(crossbow_goblin_home_positions[2], crossbow_goblin_target_ids[2]),
        CrossbowGoblin(crossbow_goblin_home_positions[3], crossbow_goblin_target_ids[3])
    }},
    _spatial_debug_overlay(stage1::data),
    _spatial_manager(stage1::data, stage1::static_obstacles, stage1::data, stage1::static_obstacles)
{
    _player.set_visible(false);
    for(Goblin& goblin : _goblins)
    {
        goblin.set_visible(false);
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.set_visible(false);
    }
}

void GameScene::enter()
{
    bn::bg_palettes::set_transparent_color(game_background_color);
    _battlefield.set_visible(true);
    _start_stage(StageId::STAGE_1);
}

void GameScene::exit()
{
    _battlefield.set_visible(false);
    _player.set_visible(false);
    for(Goblin& goblin : _goblins)
    {
        goblin.set_visible(false);
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.set_visible(false);
    }
    _crossbow_projectiles.clear();
    _hit_effects.clear();
    _collision_debug_overlay.reset();
    _spatial_debug_overlay.reset();
    _clear_stage_message();
}

bool GameScene::update()
{
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
    }

    _update_collision_debug_overlay();
    return false;
}

void GameScene::_start_stage(StageId stage)
{
    _stage = stage;
    _battlefield.set_stage(_stage == StageId::STAGE_1 ? Battlefield::StageVisual::STAGE_1 :
                           Battlefield::StageVisual::STAGE_2);
    _stage_phase = StagePhase::INTRO;
    _phase_frames_remaining = intro_frames;
    const StageData& ground_stage_data = _stage == StageId::STAGE_1 ? stage1::data : stage2::ground_data;
    const StageStaticObstacleData& ground_static_obstacles =
            _stage == StageId::STAGE_1 ? stage1::static_obstacles : stage2::ground_static_obstacles;
    const StageData& upper_stage_data = _stage == StageId::STAGE_1 ? stage1::data : stage2::upper_data;
    const StageStaticObstacleData& upper_static_obstacles =
            _stage == StageId::STAGE_1 ? stage1::static_obstacles : stage2::upper_static_obstacles;
    bn::fixed_point player_spawn = _stage == StageId::STAGE_1 ?
            bn::fixed_point(player_start_x, player_start_y) : stage2::player_spawn;
    _spatial_manager.set_stage(ground_stage_data, ground_static_obstacles, upper_stage_data, upper_static_obstacles);
    _spatial_debug_overlay.set_stage(ground_stage_data);
    _player.apply_movement(player_spawn, Direction::DOWN);
    _player.set_spatial_layer(SpatialLayer::GROUND);
    _player.set_visible(true);

    int active_goblin_count = _stage == StageId::STAGE_1 ? stage1_active_goblin_count : stage2_active_goblin_count;
    for(int index = 0; index < goblin_count; ++index)
    {
        Goblin& goblin = _goblins[index];
        if(index < active_goblin_count)
        {
            const bn::fixed_point& spawn = _stage == StageId::STAGE_1 ? goblin_home_positions[index] :
                    stage2::goblin_spawns[index];
            goblin.set_home_position(spawn);
            goblin.set_spatial_layer(SpatialLayer::GROUND);
            goblin.set_respawn_enabled(stage_enemy_respawn_enabled);
            goblin.enter();
        }
        else
        {
            goblin.deactivate();
        }
    }
    int active_crossbow_goblin_count = _stage == StageId::STAGE_1 ?
            stage1_active_crossbow_goblin_count : stage2_active_crossbow_goblin_count;
    for(int index = 0; index < crossbow_goblin_count; ++index)
    {
        CrossbowGoblin& crossbow_goblin = _crossbow_goblins[index];
        if(index < active_crossbow_goblin_count)
        {
            const bn::fixed_point& spawn = _stage == StageId::STAGE_1 ? crossbow_goblin_home_positions[index] :
                    stage2::crossbow_spawns[index];
            crossbow_goblin.set_home_position(spawn);
            crossbow_goblin.set_spatial_layer(_stage == StageId::STAGE_2 ?
                                            SpatialLayer::UPPER : SpatialLayer::GROUND);
            crossbow_goblin.set_respawn_enabled(stage_enemy_respawn_enabled);
            crossbow_goblin.enter();
        }
        else
        {
            crossbow_goblin.deactivate();
        }
    }

    _crossbow_projectiles.clear();
    _hit_effects.clear();
    _collision_debug_overlay.reset();
    _spatial_debug_overlay.reset();
    _sync_spatial_actors();
    _set_stage_message(_stage == StageId::STAGE_1 ? "STAGE 1" : "STAGE 2", 7, stage_message_y, 16, 2);
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
                        world_box(_player.position(), _player.collision_body().pushbox.box), true);

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
    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    for(int index = 0; index < goblin_count; ++index)
    {
        Goblin& goblin = _goblins[index];
        goblin.update(player_hurtbox, player_pushbox,
                goblin.spatial_layer() == _player.spatial_layer(), _spatial_manager.movement_obstacles(
                goblin_spatial_actor_ids[index], _movement_query_area(goblin.movement_obstacle_query_area())));
        _sync_spatial_actor(goblin_spatial_actor_ids[index], goblin.world_pushbox(), goblin.active());
    }
    for(int index = 0; index < crossbow_goblin_count; ++index)
    {
        CrossbowGoblin& crossbow_goblin = _crossbow_goblins[index];
        crossbow_goblin.update(player_hurtbox, player_pushbox, _spatial_manager.movement_obstacles(
                crossbow_spatial_actor_ids[index],
                _movement_query_area(crossbow_goblin.movement_obstacle_query_area())), _crossbow_projectiles);
        _sync_spatial_actor(crossbow_spatial_actor_ids[index], crossbow_goblin.world_pushbox(),
                            crossbow_goblin.active());
    }
    _crossbow_projectiles.update();

    for(Goblin& goblin : _goblins)
    {
        goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
        goblin.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    }
    for(CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.resolve_player_attack(_player.melee_attack(), _hit_effects);
    }
    _crossbow_projectiles.resolve_player_hit(_player.position(), _player.collision_body().hurtbox, _hit_effects);
    _sync_spatial_actors();
    _hit_effects.update();

    if(_all_stage_enemies_defeated())
    {
        _stage_phase = StagePhase::CLEARED;
        _set_stage_message("EXIT", 4, exit_message_y, 8, 1);
    }
}

void GameScene::_update_cleared()
{
    _update_player_gameplay();
    _hit_effects.update();

    WorldBox player_pushbox = world_box(_player.position(), _player.collision_body().pushbox.box);
    const WorldBox& exit_box = _stage == StageId::STAGE_1 ? stage1::exit_box : stage2::exit_box;
    if(touches_or_intersects(player_pushbox, exit_box))
    {
        if(_stage == StageId::STAGE_1)
        {
            _start_stage(StageId::STAGE_2);
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
                        world_box(_player.position(), _player.collision_body().pushbox.box), true);

    if(command.attack_requested)
    {
        _player.try_attack();
    }

    _player.update();
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
            bn::sprite_ptr sprite = stage_glyph_item.create_sprite(x, y, glyph_index);
            sprite.set_scale(scale);
            sprite.set_z_order(-4);
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
    for(const Goblin& goblin : _goblins)
    {
        if(goblin.active())
        {
            return false;
        }
    }

    for(const CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        if(crossbow_goblin.active())
        {
            return false;
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
    WorldBox player_hurtbox = world_box(_player.position(), _player.collision_body().hurtbox.box);
    boxes.add(player_hurtbox, CollisionDebugBoxType::HURTBOX);
    boxes.add(world_box(_player.position(), _player.collision_body().pushbox.box), CollisionDebugBoxType::PUSHBOX);
    WorldBoxList<max_hitboxes_per_frame> player_hitboxes = _player.melee_attack().active_hitboxes();
    for(int index = 0; index < player_hitboxes.count; ++index)
    {
        boxes.add(player_hitboxes.boxes[index], CollisionDebugBoxType::HITBOX);
    }
    const StageStaticObstacleData& static_obstacles = _spatial_manager.static_obstacles(_player.spatial_layer());
    for(int index = 0; index < static_obstacles.count; ++index)
    {
        boxes.add(static_obstacles.boxes[index], CollisionDebugBoxType::STATIC_OBSTACLE);
    }
    for(const Goblin& goblin : _goblins)
    {
        goblin.append_collision_debug_boxes(player_hurtbox, boxes);
    }
    bool include_crossbow_commit_boxes = _stage == StageId::STAGE_1;
    for(const CrossbowGoblin& crossbow_goblin : _crossbow_goblins)
    {
        crossbow_goblin.append_collision_debug_boxes(player_hurtbox, boxes, include_crossbow_commit_boxes);
    }
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
    for(int index = 0; index < crossbow_goblin_count; ++index)
    {
        const CrossbowGoblin& crossbow_goblin = _crossbow_goblins[index];
        _sync_spatial_actor(crossbow_spatial_actor_ids[index], crossbow_goblin.world_pushbox(),
                            crossbow_goblin.active());
    }
}

void GameScene::_sync_spatial_actor(SpatialActorId actor_id, const WorldBox& pushbox, bool active)
{
    int actor_index = int(actor_id);
    SpatialLayer layer = actor_id == SpatialActorId::PLAYER ? _player.spatial_layer() :
            actor_index >= int(SpatialActorId::CROSSBOW_GOBLIN_0) ?
                    _crossbow_goblins[actor_index - int(SpatialActorId::CROSSBOW_GOBLIN_0)].spatial_layer() :
                    _goblins[actor_index - int(SpatialActorId::GOBLIN_0)].spatial_layer();
    _spatial_manager.update_actor(actor_id, pushbox, layer);
    _spatial_manager.set_actor_active(actor_id, active);
}

WorldBox GameScene::_movement_query_area(const WorldBox& pushbox)
{
    return { pushbox.center, pushbox.width + movement_query_padding * 2,
             pushbox.height + movement_query_padding * 2 };
}
