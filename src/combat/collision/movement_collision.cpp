#include "combat/collision/movement_collision.h"

#include "combat/collision/collision_math.h"

namespace
{
    [[nodiscard]] constexpr bn::fixed clamp_position(bn::fixed position, int minimum, int maximum)
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
            const WorldBoxList<max_movement_obstacles>& obstacles, bool horizontal_axis)
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

    [[nodiscard]] constexpr bn::fixed_point resolve_horizontal_axis(
            const bn::fixed_point& current_position, const bn::fixed_point& delta,
            const Pushbox& moving_pushbox, const WorldBoxList<max_movement_obstacles>& obstacles)
    {
        WorldBox current_box = world_box(current_position, moving_pushbox.box);
        bn::fixed_point candidate(current_position.x() + delta.x(), current_position.y());
        WorldBox candidate_box = world_box(candidate, moving_pushbox.box);
        return axis_move_allowed(current_box, candidate_box, obstacles, true) ? candidate : current_position;
    }

    [[nodiscard]] constexpr bn::fixed_point resolve_vertical_axis(
            const bn::fixed_point& current_position, const bn::fixed_point& delta,
            const Pushbox& moving_pushbox, const WorldBoxList<max_movement_obstacles>& obstacles)
    {
        WorldBox current_box = world_box(current_position, moving_pushbox.box);
        bn::fixed_point candidate(current_position.x(), current_position.y() + delta.y());
        WorldBox candidate_box = world_box(candidate, moving_pushbox.box);
        return axis_move_allowed(current_box, candidate_box, obstacles, false) ? candidate : current_position;
    }

    [[nodiscard]] constexpr bool vertical_axis_has_priority(const bn::fixed_point& delta)
    {
        return collision_absolute(delta.y()) > collision_absolute(delta.x());
    }

    [[nodiscard]] constexpr bn::fixed_point resolve_movement_unbounded(
            const bn::fixed_point& current_position, const bn::fixed_point& delta,
            const Pushbox& moving_pushbox, const WorldBoxList<max_movement_obstacles>& obstacles)
    {
        // At an exact corner, either tangent axis can be clear on its own but
        // both cannot advance together. Preserve the larger intended movement
        // component, falling back to the existing X-first rule on ties.
        if(vertical_axis_has_priority(delta))
        {
            bn::fixed_point result = resolve_vertical_axis(current_position, delta, moving_pushbox, obstacles);
            return resolve_horizontal_axis(result, delta, moving_pushbox, obstacles);
        }

        bn::fixed_point result = resolve_horizontal_axis(current_position, delta, moving_pushbox, obstacles);
        return resolve_vertical_axis(result, delta, moving_pushbox, obstacles);
    }

    [[nodiscard]] constexpr bool movement_collision_tests()
    {
        WorldBoxList<max_movement_obstacles> obstacles;
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

    [[nodiscard]] constexpr bool corner_slide_tests()
    {
        constexpr Pushbox player_pushbox = { { 0, 3, 8, 8 } };
        constexpr WorldBox rock = { { 0, 0 }, 12, 12 };
        WorldBoxList<max_movement_obstacles> obstacles;
        obstacles.boxes[0] = rock;
        obstacles.count = 1;

        bn::fixed_point top_left = resolve_movement_unbounded(
                { -10, -13 }, { bn::fixed(0.5), 1 }, player_pushbox, obstacles);
        bn::fixed_point top_right = resolve_movement_unbounded(
                { 10, -13 }, { bn::fixed(-0.5), 1 }, player_pushbox, obstacles);
        bn::fixed_point bottom_left = resolve_movement_unbounded(
                { -10, 7 }, { bn::fixed(0.5), -1 }, player_pushbox, obstacles);
        bn::fixed_point bottom_right = resolve_movement_unbounded(
                { 10, 7 }, { bn::fixed(-0.5), -1 }, player_pushbox, obstacles);
        bn::fixed_point front = resolve_movement_unbounded({ -10, -3 }, { 1, 0 }, player_pushbox, obstacles);
        bn::fixed_point vertical_tangent = resolve_movement_unbounded(
                { -10, -3 }, { 0, 1 }, player_pushbox, obstacles);
        bn::fixed_point horizontal_tangent = resolve_movement_unbounded(
                { 0, -13 }, { 1, 0 }, player_pushbox, obstacles);
        bn::fixed_point free_diagonal = resolve_movement_unbounded(
                { -30, -3 }, { bn::fixed(0.5), 1 }, player_pushbox, obstacles);

        return top_left == bn::fixed_point(-10, -12) && top_right == bn::fixed_point(10, -12) &&
               bottom_left == bn::fixed_point(-10, 6) && bottom_right == bn::fixed_point(10, 6) &&
               front == bn::fixed_point(-10, -3) && vertical_tangent == bn::fixed_point(-10, -2) &&
               horizontal_tangent == bn::fixed_point(1, -13) &&
               free_diagonal == bn::fixed_point(bn::fixed(-29.5), -2) &&
               ! overlaps_strictly(world_box(top_left, player_pushbox.box), rock) &&
               ! overlaps_strictly(world_box(top_right, player_pushbox.box), rock) &&
               ! overlaps_strictly(world_box(bottom_left, player_pushbox.box), rock) &&
               ! overlaps_strictly(world_box(bottom_right, player_pushbox.box), rock);
    }

    static_assert(movement_collision_tests());
    static_assert(corner_slide_tests());
    static_assert(vertical_axis_has_priority({ bn::fixed(0.5), 1 }));
    static_assert(! vertical_axis_has_priority({ 1, 1 }));
}

bn::fixed_point resolve_movement(const bn::fixed_point& current_position, const bn::fixed_point& delta,
                                 const Pushbox& moving_pushbox, const WorldBoxList<max_movement_obstacles>& obstacles,
                                 const MovementBounds& bounds)
{
    bn::fixed_point result = resolve_movement_unbounded(current_position, delta, moving_pushbox, obstacles);

    result.set_x(clamp_position(result.x(), bounds.min_x, bounds.max_x));
    result.set_y(clamp_position(result.y(), bounds.min_y, bounds.max_y));
    return result;
}
