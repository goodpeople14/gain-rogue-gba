#include "game/game_session.h"

void GameSession::start_new_run()
{
    _current_stage = StageId::STAGE_1;
    _run_state = RunState::PLAYING;
}

StageId GameSession::current_stage() const
{
    return _current_stage;
}

GameSession::RunState GameSession::run_state() const
{
    return _run_state;
}
