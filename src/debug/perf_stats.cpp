#include "debug/perf_stats.h"

#if defined(GAIN_PERF_DEBUG_LOGS)

#include "bn_core.h"

namespace
{
    constexpr int perf_window_frames = 60;

    [[nodiscard]] int maximum(int first, int second)
    {
        return first > second ? first : second;
    }

    PerfStats stats;
}

void PerfStats::begin_frame()
{
    const int cpu_ticks = bn::core::last_cpu_ticks();
    const int vblank_ticks = bn::core::last_vblank_ticks();
    const int missed_frames = bn::core::last_missed_frames();
    cpu_ticks_total += cpu_ticks;
    cpu_ticks_max = maximum(cpu_ticks_max, cpu_ticks);
    vblank_ticks_max = maximum(vblank_ticks_max, vblank_ticks);
    missed_frames_total += missed_frames;
    missed_frames_max = maximum(missed_frames_max, missed_frames);
}

bool PerfStats::finish_frame()
{
    ++frame_count;
    return frame_count == perf_window_frames;
}

void PerfStats::reset()
{
    *this = {};
}

PerfStats& perf_stats()
{
    return stats;
}

#endif
