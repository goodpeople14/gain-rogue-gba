#ifndef GAME_SESSION_H
#define GAME_SESSION_H

#include "world/stage_definition.h"

class GameSession
{
public:
    enum class RunState
    {
        PLAYING,
        CLEARED,
        FAILED
    };

    void start_new_run();

    [[nodiscard]] StageId current_stage() const;
    [[nodiscard]] RunState run_state() const;

private:
    StageId _current_stage = StageId::STAGE_1;
    RunState _run_state = RunState::PLAYING;
};

#endif
