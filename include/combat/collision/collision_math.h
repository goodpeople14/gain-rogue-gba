#ifndef COLLISION_COLLISION_MATH_H
#define COLLISION_COLLISION_MATH_H

#include "combat/collision/collision_box.h"

[[nodiscard]] constexpr bn::fixed collision_absolute(bn::fixed value)
{
    return value < 0 ? -value : value;
}

[[nodiscard]] constexpr bool touches_or_intersects(const WorldBox& first, const WorldBox& second)
{
    return collision_absolute(first.center.x() - second.center.x()) * 2 <= first.width + second.width &&
           collision_absolute(first.center.y() - second.center.y()) * 2 <= first.height + second.height;
}

[[nodiscard]] constexpr bool overlaps_strictly(const WorldBox& first, const WorldBox& second)
{
    return collision_absolute(first.center.x() - second.center.x()) * 2 < first.width + second.width &&
           collision_absolute(first.center.y() - second.center.y()) * 2 < first.height + second.height;
}

[[nodiscard]] constexpr bn::fixed horizontal_overlap(const WorldBox& first, const WorldBox& second)
{
    bn::fixed result = bn::fixed(first.width + second.width) / 2 -
            collision_absolute(first.center.x() - second.center.x());
    return result > 0 ? result : bn::fixed(0);
}

[[nodiscard]] constexpr bn::fixed vertical_overlap(const WorldBox& first, const WorldBox& second)
{
    bn::fixed result = bn::fixed(first.height + second.height) / 2 -
            collision_absolute(first.center.y() - second.center.y());
    return result > 0 ? result : bn::fixed(0);
}

#endif
