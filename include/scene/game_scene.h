#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "bn_array.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_vector.h"

#include "character/player_controller.h"
#include "character/swordsman.h"
#include "enemy/enemy_runtime.h"
#include "combat/crossbow_projectile_pool.h"
#include "combat/hit_effect_manager.h"
#include "debug/collision_debug_overlay.h"
#include "debug/spatial_debug_overlay.h"
#include "world/battlefield.h"
#include "world/spatial_manager.h"

class GameScene
{
public:
    static constexpr int goblin_count = 4;
    static constexpr int crossbow_goblin_count = 4;
    static constexpr int enemy_count = goblin_count + crossbow_goblin_count;

    enum class StagePhase
    {
        INTRO,
        READY,
        GO,
        PLAYING,
        CLEARED,
        PLAYER_DEAD,
        CONGRATULATIONS,
        GAME_OVER
    };

    GameScene();

    void enter();
    void exit();
    [[nodiscard]] bool update();

private:
    static constexpr int max_player_hud_name_length = 16;
    static constexpr int max_player_health_hud_cells = 8;

    enum class StageId
    {
        STAGE_1,
        STAGE_2
    };

    void _start_stage(StageId stage);
    void _update_stage_phase();
    void _update_playing();
    void _update_cleared();
    void _update_player_gameplay();
    void _apply_player_damage(int damage);
    void _update_player_health_hud();
    void _set_player_hud_visible(bool visible);
    void _set_stage_message(const char* text, int character_count, int y, int spacing, bn::fixed scale);
    void _clear_stage_message();
    [[nodiscard]] bool _stage_has_enemies() const;
    [[nodiscard]] bool _all_stage_enemies_defeated() const;
    void _update_collision_debug_overlay();
    void _cycle_debug_enemy_type(int offset);
    void _cycle_debug_enemy_representative();
    void _validate_debug_enemy_type();
    [[nodiscard]] bn::vector<CharacterId, enemy_count> _active_debug_enemy_types() const;
    [[nodiscard]] Enemy* _first_debug_enemy_of_type(CharacterId type);
    [[nodiscard]] Enemy* _debug_enemy_representative();
    void _sync_spatial_actors();
    void _sync_spatial_actor(SpatialActorId actor_id, const WorldBox& pushbox,
                             SpatialLayer layer, bool active);
    [[nodiscard]] static WorldBox _movement_query_area(const WorldBox& pushbox);

    Battlefield _battlefield;
    Swordsman _player;
    PlayerController _player_controller;
    MovementBounds _player_bounds;
    EnemyRuntime _enemy_runtime;
    CrossbowProjectilePool _crossbow_projectiles;
    HitEffectManager _hit_effects;
    bn::array<bn::sprite_tiles_ptr, 20> _stage_glyph_tiles;
    bn::sprite_palette_ptr _stage_glyph_palette;
    bn::array<bn::sprite_tiles_ptr, 2> _player_health_tiles;
    bn::vector<bn::sprite_ptr, max_player_hud_name_length> _player_name_sprites;
    bn::vector<bn::sprite_ptr, max_player_health_hud_cells> _player_health_sprites;
    CollisionDebugOverlay _collision_debug_overlay;
    SpatialDebugOverlay _spatial_debug_overlay;
    SpatialManager _spatial_manager;
    bn::vector<bn::sprite_ptr, 20> _stage_message_sprites;
    StageId _stage = StageId::STAGE_1;
    StagePhase _stage_phase = StagePhase::INTRO;
    bn::optional<CharacterId> _debug_enemy_type;
    bn::optional<int> _debug_enemy_actor_id;
    int _phase_frames_remaining = 0;

#if defined(GAIN_DEBUG_LOGS) || defined(GAIN_PERF_DEBUG_LOGS)
    int _debug_log_frame_count = 0;
#endif
};

#endif
