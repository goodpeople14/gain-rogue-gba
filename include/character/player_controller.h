#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "bn_fixed_point.h"
#include "character/character.h"
#include "game/direction.h"

struct MovementIntent
{
    bn::fixed_point delta;
    Direction direction;
    bool moving;
};

class PlayerController
{
public:
    [[nodiscard]] MovementIntent movement_intent(const Character& character) const;

private:
    [[nodiscard]] static Direction _direction_from_input(int horizontal, int vertical);
};

#endif
