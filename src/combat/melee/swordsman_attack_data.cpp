#include "combat/melee/swordsman_attack_data.h"

#include "combat/collision/collision_math.h"

namespace
{
    using DirectionFrames = bn::array<AttackFrameData, swordsman_attack_game_frames>;

    [[nodiscard]] constexpr Hitbox hitbox(int x, int y)
    {
        return {{ x, y, 8, 8 }};
    }

    [[nodiscard]] constexpr AttackFrameData inactive_frame(int image_frame)
    {
        return { {}, 0, image_frame };
    }

    [[nodiscard]] constexpr AttackFrameData active_frame(int image_frame, Hitbox first, Hitbox second)
    {
        return {{{ first, second }}, 2, image_frame};
    }

    [[nodiscard]] constexpr DirectionFrames direction_frames(
            Hitbox early_first, Hitbox early_second, Hitbox middle_first, Hitbox middle_second,
            Hitbox late_first, Hitbox late_second)
    {
        return {{
            inactive_frame(1),
            active_frame(2, early_first, early_second),
            active_frame(3, middle_first, middle_second),
            active_frame(4, late_first, late_second),
            active_frame(4, late_first, late_second),
            inactive_frame(5)
        }};
    }

    constexpr bn::array<DirectionFrames, 8> attack_table = {{
        direction_frames(hitbox(-9, 8), hitbox(-15, 13), hitbox(0, 12), hitbox(0, 20),
                         hitbox(9, 8), hitbox(15, 13)),
        direction_frames(hitbox(-12, -1), hitbox(-20, -1), hitbox(-8, 8), hitbox(-14, 14),
                         hitbox(1, 12), hitbox(1, 20)),
        direction_frames(hitbox(-8, -9), hitbox(-13, -15), hitbox(-12, 0), hitbox(-20, 0),
                         hitbox(-8, 9), hitbox(-13, 15)),
        direction_frames(hitbox(1, -12), hitbox(1, -20), hitbox(-8, -8), hitbox(-14, -14),
                         hitbox(-12, 1), hitbox(-20, 1)),
        direction_frames(hitbox(9, -8), hitbox(15, -13), hitbox(0, -12), hitbox(0, -20),
                         hitbox(-9, -8), hitbox(-15, -13)),
        direction_frames(hitbox(12, 1), hitbox(20, 1), hitbox(8, -8), hitbox(14, -14),
                         hitbox(-1, -12), hitbox(-1, -20)),
        direction_frames(hitbox(8, 9), hitbox(13, 15), hitbox(12, 0), hitbox(20, 0),
                         hitbox(8, -9), hitbox(13, -15)),
        direction_frames(hitbox(-1, 12), hitbox(-1, 20), hitbox(8, 8), hitbox(14, 14),
                         hitbox(12, -1), hitbox(20, -1))
    }};

    [[nodiscard]] constexpr int integer_absolute(int value)
    {
        return value < 0 ? -value : value;
    }

    [[nodiscard]] constexpr bool frame_hits(Direction direction, int game_frame,
                                            const bn::fixed_point& target_position)
    {
        const AttackFrameData& frame = attack_table[int(direction)][game_frame - 1];
        constexpr Hurtbox test_hurtbox = {{ 0, 0, 10, 10 }};
        WorldBox target = world_box(target_position, test_hurtbox.box);

        for(int index = 0; index < frame.hitbox_count; ++index)
        {
            if(touches_or_intersects(world_box({ 0, 0 }, frame.hitboxes[index].box), target))
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] constexpr bool frame_table_tests()
    {
        for(const DirectionFrames& frames : attack_table)
        {
            if(frames[0].hitbox_count != 0 || frames[1].hitbox_count != 2 ||
               frames[2].hitbox_count != 2 || frames[3].hitbox_count != 2 ||
               frames[4].hitbox_count != 2 || frames[5].hitbox_count != 0 ||
               frames[0].image_frame != 1 || frames[1].image_frame != 2 ||
               frames[2].image_frame != 3 || frames[3].image_frame != 4 ||
               frames[4].image_frame != 4 || frames[5].image_frame != 5)
            {
                return false;
            }
        }

        bool opposite_symmetry = true;
        int maximum_center_reach = 0;

        for(int direction = 0; direction < 8; ++direction)
        {
            for(int frame = 0; frame < swordsman_attack_game_frames; ++frame)
            {
                const AttackFrameData& data = attack_table[direction][frame];

                for(int index = 0; index < data.hitbox_count; ++index)
                {
                    const LocalBox& box = data.hitboxes[index].box;
                    int reach = integer_absolute(box.offset_x) > integer_absolute(box.offset_y) ?
                            integer_absolute(box.offset_x) : integer_absolute(box.offset_y);

                    if(reach > maximum_center_reach)
                    {
                        maximum_center_reach = reach;
                    }
                }
            }
        }

        for(int direction = 0; direction < 4; ++direction)
        {
            for(int frame = 0; frame < swordsman_attack_game_frames; ++frame)
            {
                const AttackFrameData& first = attack_table[direction][frame];
                const AttackFrameData& opposite = attack_table[direction + 4][frame];

                for(int index = 0; index < first.hitbox_count; ++index)
                {
                    opposite_symmetry = opposite_symmetry &&
                            first.hitboxes[index].box.offset_x == -opposite.hitboxes[index].box.offset_x &&
                            first.hitboxes[index].box.offset_y == -opposite.hitboxes[index].box.offset_y;
                }
            }
        }

        return opposite_symmetry && maximum_center_reach == 20 &&
               attack_table[int(Direction::DOWN)][1].hitboxes[0].box.offset_x == -9 &&
               attack_table[int(Direction::DOWN)][2].hitboxes[0].box.offset_x == 0 &&
               attack_table[int(Direction::DOWN)][3].hitboxes[0].box.offset_x == 9 &&
               attack_table[int(Direction::UP)][2].hitboxes[1].box.offset_y == -20 &&
               attack_table[int(Direction::LEFT)][2].hitboxes[1].box.offset_x == -20 &&
               attack_table[int(Direction::RIGHT)][2].hitboxes[1].box.offset_x == 20 &&
               ! frame_hits(Direction::DOWN, 1, { 0, 12 }) &&
               frame_hits(Direction::DOWN, 2, { -9, 8 }) &&
               frame_hits(Direction::DOWN, 3, { 0, 29 }) &&
               ! frame_hits(Direction::DOWN, 3, { 0, 30 }) &&
               ! frame_hits(Direction::DOWN_LEFT, 3, { -22, 0 }) &&
               ! frame_hits(Direction::DOWN, 6, { 0, 20 });
    }

    static_assert(frame_table_tests());
}

const AttackFrameData& swordsman_attack_frame_data(Direction direction, int game_frame)
{
    return attack_table[int(direction)][game_frame - 1];
}
