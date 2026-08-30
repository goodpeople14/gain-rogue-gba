#include "game/game_session.h"

void GameSession::start_new_run()
{
    _current_stage = StageId::STAGE_1;
    _run_state = RunState::PLAYING;
}

bool GameSession::complete_current_stage()
{
    switch(_current_stage)
    {
    case StageId::STAGE_1:
        _current_stage = StageId::STAGE_2;
        return true;

    case StageId::STAGE_2:
        _current_stage = StageId::STAGE_3;
        return true;

    case StageId::STAGE_3:
    case StageId::STAGE_4:
    case StageId::STAGE_5:
        return false;

    default:
        return false;
    }
}

StageId GameSession::current_stage() const
{
    return _current_stage;
}

GameSession::RunState GameSession::run_state() const
{
    return _run_state;
}
