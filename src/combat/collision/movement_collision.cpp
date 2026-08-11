#include "combat/collision/movement_collision.h"

#include "combat/collision/collision_math.h"
#include "world/stages/stage1.h"

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

    [[nodiscard]] constexpr bool rock_inset_tests()
    {
        constexpr int rock_visual_size = 16;
        constexpr int rock_collision_size = 10;
        constexpr Pushbox player_pushbox = { { 0, 3, 8, 8 } };
        constexpr WorldBox rock_visual = { { 0, 0 }, rock_visual_size, rock_visual_size };
        constexpr WorldBox rock_collision = { { 0, 0 }, rock_collision_size, rock_collision_size };
        constexpr bn::fixed diagonal_step(0.70710678f);
        WorldBoxList<max_movement_obstacles> obstacles;
        obstacles.boxes[0] = rock_collision;
        obstacles.count = 1;

        // The sprite overlaps the rounded visual corner, while the actual
        // pushbox remains outside the 3px-inset collision box.
        constexpr bn::fixed_point corner_clearance_start(-10, -10);
        bn::fixed_point corner_clearance = resolve_movement_unbounded(
                corner_clearance_start, { diagonal_step, diagonal_step }, player_pushbox, obstacles);

        // Moving one pixel closer reaches the collision box; X is blocked and
        // the existing dominant-axis tie rule preserves the tangent Y slide.
        constexpr bn::fixed_point entering_start(-9, -10);
        bn::fixed_point entering_collision = resolve_movement_unbounded(
                entering_start, { diagonal_step, diagonal_step }, player_pushbox, obstacles);

        return rock_visual.width - rock_collision.width == 6 &&
               rock_visual.height - rock_collision.height == 6 &&
               overlaps_strictly({ corner_clearance_start, rock_visual_size, rock_visual_size }, rock_visual) &&
               ! overlaps_strictly(world_box(corner_clearance_start, player_pushbox.box), rock_collision) &&
               corner_clearance == bn::fixed_point(-10 + diagonal_step, -10 + diagonal_step) &&
               entering_collision == bn::fixed_point(-9, -10 + diagonal_step) &&
               ! overlaps_strictly(world_box(entering_collision, player_pushbox.box), rock_collision);
    }

    [[nodiscard]] constexpr bool rock_collision_candidate_passes(int collision_size)
    {
        constexpr int rock_visual_size = 16;
        constexpr Pushbox player_pushbox = { { 0, 3, 8, 8 } };
        constexpr bn::fixed diagonal_step(0.70710678f);
        WorldBoxList<max_movement_obstacles> obstacles;
        obstacles.boxes[0] = { { 0, 0 }, collision_size, collision_size };
        obstacles.count = 1;

        bn::fixed_point top_left = resolve_movement_unbounded(
                { -10, -10 }, { diagonal_step, diagonal_step }, player_pushbox, obstacles);
        bn::fixed_point top_right = resolve_movement_unbounded(
                { 10, -10 }, { -diagonal_step, diagonal_step }, player_pushbox, obstacles);
        bn::fixed_point bottom_left = resolve_movement_unbounded(
                { -10, 10 }, { diagonal_step, -diagonal_step }, player_pushbox, obstacles);
        bn::fixed_point bottom_right = resolve_movement_unbounded(
                { 10, 10 }, { -diagonal_step, -diagonal_step }, player_pushbox, obstacles);
        bn::fixed_point left_side = resolve_movement_unbounded(
                { -(collision_size / 2) - 4, -3 }, { 1, 0 }, player_pushbox, obstacles);
        bn::fixed_point right_side = resolve_movement_unbounded(
                { (collision_size / 2) + 4, -3 }, { -1, 0 }, player_pushbox, obstacles);
        bn::fixed_point top_side = resolve_movement_unbounded(
                { 0, -(collision_size / 2) - 7 }, { 0, 1 }, player_pushbox, obstacles);
        bn::fixed_point bottom_side = resolve_movement_unbounded(
                { 0, (collision_size / 2) + 1 }, { 0, -1 }, player_pushbox, obstacles);

        return top_left == bn::fixed_point(-10 + diagonal_step, -10 + diagonal_step) &&
               top_right == bn::fixed_point(10 - diagonal_step, -10 + diagonal_step) &&
               bottom_left == bn::fixed_point(-10 + diagonal_step, 10 - diagonal_step) &&
               bottom_right == bn::fixed_point(10 - diagonal_step, 10 - diagonal_step) &&
               left_side.x() == -(collision_size / 2) - 4 &&
               right_side.x() == (collision_size / 2) + 4 &&
               top_side.y() == -(collision_size / 2) - 7 &&
               bottom_side.y() == (collision_size / 2) + 1 &&
               ! overlaps_strictly(world_box(left_side, player_pushbox.box), obstacles.boxes[0]) &&
               ! overlaps_strictly(world_box(right_side, player_pushbox.box), obstacles.boxes[0]) &&
               ! overlaps_strictly(world_box(top_side, player_pushbox.box), obstacles.boxes[0]) &&
               ! overlaps_strictly(world_box(bottom_side, player_pushbox.box), obstacles.boxes[0]) &&
               rock_visual_size > collision_size;
    }

    [[nodiscard]] constexpr int largest_passing_rock_collision_size()
    {
        return rock_collision_candidate_passes(12) ? 12 :
               rock_collision_candidate_passes(10) ? 10 :
               rock_collision_candidate_passes(8) ? 8 : 0;
    }

    static_assert(movement_collision_tests());
    static_assert(corner_slide_tests());
    static_assert(rock_inset_tests());
    static_assert(! rock_collision_candidate_passes(12));
    static_assert(rock_collision_candidate_passes(10));
    static_assert(rock_collision_candidate_passes(8));
    static_assert(largest_passing_rock_collision_size() == 10);
    static_assert(stage1::rock_collision_size == largest_passing_rock_collision_size());
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
