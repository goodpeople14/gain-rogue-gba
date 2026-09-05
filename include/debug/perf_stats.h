#ifndef DEBUG_PERF_STATS_H
#define DEBUG_PERF_STATS_H

#if defined(GAIN_PERF_DEBUG_LOGS)

class PerfStats
{
public:
    void begin_frame();
    [[nodiscard]] bool finish_frame();
    void reset();

    int frame_count = 0;
    int cpu_ticks_total = 0;
    int cpu_ticks_max = 0;
    int vblank_ticks_max = 0;
    int missed_frames_total = 0;
    int missed_frames_max = 0;
    int movement_query_calls = 0;
    int stationary_query_calls = 0;
    int inactive_query_calls = 0;
    int spatial_sync_calls = 0;
    int spatial_sync_noops = 0;
    int position_remove_calls = 0;
    int position_register_calls = 0;
    int stage_cells_examined = 0;
    int position_cells_examined = 0;
    int candidate_actor_entries = 0;
    int result_obstacle_total = 0;
    int result_obstacle_max = 0;
    int resolve_movement_calls = 0;
    int movement_full = 0;
    int movement_partial = 0;
    int movement_blocked = 0;
    int detour_start_count = 0;
    int detour_candidate_checks = 0;
    int crossbow_telegraph_frames = 0;
    int crossbow_telegraph_apply_movement_calls = 0;
    int crossbow_direction_changes = 0;
    int status_icon_position_updates = 0;
    int projectile_spawn_attempts = 0;
    int projectile_spawn_success = 0;
    int projectile_spawn_dropped_pool_full = 0;
    int active_projectile_max = 0;
};

[[nodiscard]] PerfStats& perf_stats();

#endif

#endif
