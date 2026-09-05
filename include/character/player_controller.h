#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "character/character.h"
#include "character/movement_intent.h"

struct PlayerCommand
{
    MovementIntent movement;
    bool attack_requested;
};

class PlayerController
{
public:
    [[nodiscard]] PlayerCommand command(const Character& character) const;

private:
    [[nodiscard]] MovementIntent _movement_intent(const Character& character) const;
    [[nodiscard]] static Direction _direction_from_input(int horizontal, int vertical);
};

#endif
