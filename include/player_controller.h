#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "battlefield.h"
#include "character.h"
#include "direction.h"

class PlayerController
{
public:
    void update(Character& character, const MovementBounds& bounds) const;

private:
    [[nodiscard]] static Direction _direction_from_input(int horizontal, int vertical);
};

#endif
