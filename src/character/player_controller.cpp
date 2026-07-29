#include "character/player_controller.h"

#include "bn_keypad.h"

namespace
{
    constexpr bn::fixed diagonal_movement_ratio(0.70710678f);
}

MovementIntent PlayerController::movement_intent(const Character& character) const
{
    int horizontal = int(bn::keypad::right_held()) - int(bn::keypad::left_held());
    int vertical = int(bn::keypad::down_held()) - int(bn::keypad::up_held());

    if(horizontal || vertical)
    {
        Direction direction = _direction_from_input(horizontal, vertical);
        bn::fixed movement_speed = character.movement_speed();

        if(horizontal && vertical)
        {
            movement_speed *= diagonal_movement_ratio;
        }

        return { bn::fixed_point(horizontal * movement_speed, vertical * movement_speed), direction, true };
    }

    return { bn::fixed_point(0, 0), character.direction(), false };
}

Direction PlayerController::_direction_from_input(int horizontal, int vertical)
{
    if(vertical > 0)
    {
        if(horizontal < 0)
        {
            return Direction::DOWN_LEFT;
        }

        if(horizontal > 0)
        {
            return Direction::DOWN_RIGHT;
        }

        return Direction::DOWN;
    }

    if(vertical < 0)
    {
        if(horizontal < 0)
        {
            return Direction::UP_LEFT;
        }

        if(horizontal > 0)
        {
            return Direction::UP_RIGHT;
        }

        return Direction::UP;
    }

    return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
}
