#include "game/game_session.h"

void GameSession::start_new_run()
{
    _current_stage = StageId::STAGE_1;
    _run_state = RunState::PLAYING;
}

void GameSession::complete_current_stage()
{
#if defined(GAIN_PERF_DEBUG_LOGS)
    if(_current_stage == StageId::STAGE_5)
#else
    if(_current_stage == StageId::STAGE_3)
#endif
    {
        _run_state = RunState::CLEARED;
    }
}

bool GameSession::advance_after_stage_result()
{
    switch(_current_stage)
    {
    case StageId::STAGE_1:
#if defined(GAIN_PERF_DEBUG_LOGS)
        _current_stage = StageId::STAGE_3;
#else
        _current_stage = StageId::STAGE_2;
#endif
        return true;

    case StageId::STAGE_2:
        _current_stage = StageId::STAGE_3;
        return true;

    case StageId::STAGE_3:
#if defined(GAIN_PERF_DEBUG_LOGS)
        _current_stage = StageId::STAGE_4;
        return true;
#else
        return false;
#endif

    case StageId::STAGE_4:
#if defined(GAIN_PERF_DEBUG_LOGS)
        _current_stage = StageId::STAGE_5;
        return true;
#else
        return false;
#endif

    case StageId::STAGE_5:
        return false;

    default:
        return false;
    }
}

void GameSession::fail_run()
{
    _run_state = RunState::FAILED;
}

StageId GameSession::current_stage() const
{
    return _current_stage;
}

GameSession::RunState GameSession::run_state() const
{
    return _run_state;
}
