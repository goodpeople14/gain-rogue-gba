#include "combat/collision/movement_collision.h"

#include "combat/collision/collision_math.h"

namespace
{
    [[nodiscard]] bn::fixed clamp_position(bn::fixed position, int minimum, int maximum)
    {
        if(position < minimum)
        {
            return minimum;
        }

        if(position > maximum)
        {
            return maximum;
        }

        return position;
    }

    [[nodiscard]] constexpr bool axis_move_allowed(
            const WorldBox& current_box, const WorldBox& candidate_box,
            const WorldBoxList<3>& obstacles, bool horizontal_axis)
    {
        for(int index = 0; index < obstacles.count; ++index)
        {
            const WorldBox& obstacle = obstacles.boxes[index];

            if(overlaps_strictly(candidate_box, obstacle))
            {
                if(! overlaps_strictly(current_box, obstacle))
                {
                    return false;
                }

                bn::fixed current_overlap = horizontal_axis ? horizontal_overlap(current_box, obstacle) :
                        vertical_overlap(current_box, obstacle);
                bn::fixed candidate_overlap = horizontal_axis ? horizontal_overlap(candidate_box, obstacle) :
                        vertical_overlap(candidate_box, obstacle);

                if(candidate_overlap >= current_overlap)
                {
                    return false;
                }
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool movement_collision_tests()
    {
        WorldBoxList<3> obstacles;
        obstacles.boxes[0] = { { 0, 0 }, 10, 10 };
        obstacles.count = 1;

        WorldBox touching = { { 0, 9 }, 8, 8 };
        WorldBox newly_overlapping = { { 0, 8 }, 8, 8 };
        WorldBox overlapping = { { 0, 7 }, 8, 8 };
        WorldBox less_overlapping = { { 0, 8 }, 8, 8 };
        WorldBox more_overlapping = { { 0, 6 }, 8, 8 };

        return ! overlaps_strictly(touching, obstacles.boxes[0]) &&
               ! axis_move_allowed(touching, newly_overlapping, obstacles, false) &&
               axis_move_allowed(overlapping, less_overlapping, obstacles, false) &&
               ! axis_move_allowed(overlapping, more_overlapping, obstacles, false);
    }

    static_assert(movement_collision_tests());
}

bn::fixed_point resolve_movement(const bn::fixed_point& current_position, const bn::fixed_point& delta,
                                 const Pushbox& moving_pushbox, const WorldBoxList<3>& obstacles,
                                 const MovementBounds& bounds)
{
    bn::fixed_point result = current_position;
    WorldBox current_box = world_box(result, moving_pushbox.box);
    bn::fixed_point x_candidate(result.x() + delta.x(), result.y());
    WorldBox x_candidate_box = world_box(x_candidate, moving_pushbox.box);

    if(axis_move_allowed(current_box, x_candidate_box, obstacles, true))
    {
        result.set_x(x_candidate.x());
    }

    current_box = world_box(result, moving_pushbox.box);
    bn::fixed_point y_candidate(result.x(), result.y() + delta.y());
    WorldBox y_candidate_box = world_box(y_candidate, moving_pushbox.box);

    if(axis_move_allowed(current_box, y_candidate_box, obstacles, false))
    {
        result.set_y(y_candidate.y());
    }

    result.set_x(clamp_position(result.x(), bounds.min_x, bounds.max_x));
    result.set_y(clamp_position(result.y(), bounds.min_y, bounds.max_y));
    return result;
}
