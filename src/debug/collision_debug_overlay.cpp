#include "debug/collision_debug_overlay.h"

#include "bn_assert.h"
#include "bn_sprite_items_collision_debug_commit_line_6.h"
#include "bn_sprite_items_collision_debug_commit_line_8.h"
#include "bn_sprite_items_collision_debug_commit_line_10.h"
#include "bn_sprite_items_collision_debug_commit_line_12.h"
#include "bn_sprite_items_collision_debug_commit_line_16.h"
#include "bn_sprite_items_collision_debug_commit_line_32.h"
#include "bn_sprite_items_collision_debug_commit_line_64.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_6.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_8.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_10.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_12.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_16.h"
#include "bn_sprite_items_collision_debug_commit_vertical_line_32.h"
#include "bn_sprite_items_collision_debug_hit_line_6.h"
#include "bn_sprite_items_collision_debug_hit_line_8.h"
#include "bn_sprite_items_collision_debug_hit_line_10.h"
#include "bn_sprite_items_collision_debug_hit_line_12.h"
#include "bn_sprite_items_collision_debug_hit_line_16.h"
#include "bn_sprite_items_collision_debug_hit_line_32.h"
#include "bn_sprite_items_collision_debug_hit_line_64.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_6.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_8.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_10.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_12.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_16.h"
#include "bn_sprite_items_collision_debug_hit_vertical_line_32.h"
#include "bn_sprite_items_collision_debug_hurt_line_6.h"
#include "bn_sprite_items_collision_debug_hurt_line_8.h"
#include "bn_sprite_items_collision_debug_hurt_line_10.h"
#include "bn_sprite_items_collision_debug_hurt_line_12.h"
#include "bn_sprite_items_collision_debug_hurt_line_16.h"
#include "bn_sprite_items_collision_debug_hurt_line_32.h"
#include "bn_sprite_items_collision_debug_hurt_line_64.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_6.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_8.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_10.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_12.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_16.h"
#include "bn_sprite_items_collision_debug_hurt_vertical_line_32.h"
#include "bn_sprite_items_collision_debug_push_line_6.h"
#include "bn_sprite_items_collision_debug_push_line_8.h"
#include "bn_sprite_items_collision_debug_push_line_10.h"
#include "bn_sprite_items_collision_debug_push_line_12.h"
#include "bn_sprite_items_collision_debug_push_line_16.h"
#include "bn_sprite_items_collision_debug_push_line_32.h"
#include "bn_sprite_items_collision_debug_push_line_64.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_6.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_8.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_10.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_12.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_16.h"
#include "bn_sprite_items_collision_debug_push_vertical_line_32.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_6.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_8.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_10.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_12.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_16.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_32.h"
#include "bn_sprite_items_collision_debug_static_obstacle_line_64.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_6.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_8.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_10.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_12.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_16.h"
#include "bn_sprite_items_collision_debug_static_obstacle_vertical_line_32.h"

namespace
{
    enum class GuideStyle
    {
        HURT,
        HIT,
        PUSH,
        STATIC_OBSTACLE,
        COMMIT
    };

    enum class SegmentLength
    {
        SIX = 6,
        EIGHT = 8,
        TEN = 10,
        TWELVE = 12,
        SIXTEEN = 16,
        THIRTY_TWO = 32,
        SIXTY_FOUR = 64
    };

    struct RasterRectangle
    {
        int left;
        int right;
        int top;
        int bottom;

        [[nodiscard]] constexpr int width() const { return right - left; }
        [[nodiscard]] constexpr int height() const { return bottom - top; }
    };

    // floor(radius / sqrt(2)) with a fixed rational approximation.  The four
    // corners of this guide remain inside the actual Euclidean AI radius.
    [[nodiscard]] constexpr int radius_guide_half_extent(int radius)
    {
        return (radius * 181) / 256;
    }

    [[nodiscard]] constexpr bool guide_corners_are_inside_radius(int radius)
    {
        int half_extent = radius_guide_half_extent(radius);
        return 2 * half_extent * half_extent <= radius * radius;
    }

    static_assert(radius_guide_half_extent(48) == 33);
    static_assert(radius_guide_half_extent(72) == 50);
    static_assert(radius_guide_half_extent(96) == 67);
    static_assert(radius_guide_half_extent(110) == 77);
    static_assert(guide_corners_are_inside_radius(48));
    static_assert(guide_corners_are_inside_radius(72));
    static_assert(guide_corners_are_inside_radius(96));
    static_assert(guide_corners_are_inside_radius(110));

    [[nodiscard]] constexpr RasterRectangle raster_rectangle(const WorldBox& box)
    {
        bn::fixed half_width = bn::fixed(box.width) / 2;
        bn::fixed half_height = bn::fixed(box.height) / 2;
        return { (box.center.x() - half_width).shift_integer(),
                 (box.center.x() + half_width).shift_integer(),
                 (box.center.y() - half_height).shift_integer(),
                 (box.center.y() + half_height).shift_integer() };
    }

    [[nodiscard]] constexpr RasterRectangle radius_guide_rectangle(const CollisionDebugRadius& radius)
    {
        int half_extent = radius_guide_half_extent(radius.radius);
        return { radius.center.x().shift_integer() - half_extent,
                 radius.center.x().shift_integer() + half_extent,
                 radius.center.y().shift_integer() - half_extent,
                 radius.center.y().shift_integer() + half_extent };
    }

    [[nodiscard]] constexpr int largest_physical_segment(int remaining)
    {
        if(remaining >= 32) return 32;
        if(remaining >= 16) return 16;
        if(remaining >= 12) return 12;
        if(remaining >= 10) return 10;
        if(remaining >= 8) return 8;
        return 6;
    }

    [[nodiscard]] constexpr int largest_radius_segment(int remaining, bool vertical)
    {
        if(! vertical && remaining >= 64) return 64;
        if(remaining >= 32) return 32;
        return 16;
    }

    [[nodiscard]] constexpr int fitted_extent(int extent, bool radius_guide, bool vertical)
    {
        int fitted = 0;
        int remaining = extent;
        while(remaining >= (radius_guide ? 16 : 6))
        {
            int segment = radius_guide ? largest_radius_segment(remaining, vertical) :
                                         largest_physical_segment(remaining);
            if(segment > remaining)
            {
                break;
            }
            fitted += segment;
            remaining -= segment;
        }
        return fitted;
    }

    [[nodiscard]] constexpr RasterRectangle fitted_rectangle(
            RasterRectangle rectangle, bool radius_guide)
    {
        int fitted_width = fitted_extent(rectangle.width(), radius_guide, false);
        int fitted_height = fitted_extent(rectangle.height(), radius_guide, true);
        int left_inset = (rectangle.width() - fitted_width) / 2;
        int top_inset = (rectangle.height() - fitted_height) / 2;
        return { rectangle.left + left_inset, rectangle.left + left_inset + fitted_width,
                 rectangle.top + top_inset, rectangle.top + top_inset + fitted_height };
    }

    constexpr RasterRectangle test_rectangle_16 = fitted_rectangle({ -8, 8, -8, 8 }, false);
    static_assert(test_rectangle_16.width() == 16 && test_rectangle_16.height() == 16);
    constexpr RasterRectangle test_rectangle_wide = fitted_rectangle({ -32, 32, -8, 8 }, false);
    static_assert(test_rectangle_wide.width() == 64 && test_rectangle_wide.height() == 16);
    constexpr RasterRectangle test_rectangle_tall = fitted_rectangle({ -8, 8, -32, 32 }, false);
    static_assert(test_rectangle_tall.width() == 16 && test_rectangle_tall.height() == 64);
    constexpr RasterRectangle test_rectangle_negative = fitted_rectangle({ -8, 8, -80, -16 }, false);
    static_assert(test_rectangle_negative.left == -8 && test_rectangle_negative.top == -80);
    static_assert(test_rectangle_negative.right == 8 && test_rectangle_negative.bottom == -16);
    static_assert(fitted_extent(66, true, false) == 64);
    static_assert(fitted_extent(66, true, true) == 64);
    static_assert(fitted_extent(100, true, false) == 96);
    static_assert(fitted_extent(100, true, true) == 96);
    static_assert(fitted_extent(134, true, false) == 128);
    static_assert(fitted_extent(134, true, true) == 128);
    static_assert(fitted_extent(154, true, false) == 144);
    static_assert(fitted_extent(154, true, true) == 144);
    static_assert(2 * 32 * 32 <= 48 * 48);
    static_assert(2 * 48 * 48 <= 72 * 72);
    static_assert(2 * 64 * 64 <= 96 * 96);
    static_assert(2 * 72 * 72 <= 110 * 110);

    [[nodiscard]] constexpr int edge_segment_count(int extent, bool radius_guide, bool vertical)
    {
        int count = 0;
        int remaining = fitted_extent(extent, radius_guide, vertical);
        while(remaining > 0)
        {
            int segment = radius_guide ? largest_radius_segment(remaining, vertical) :
                                         largest_physical_segment(remaining);
            if(segment > remaining)
            {
                break;
            }
            ++count;
            remaining -= segment;
        }
        return count;
    }

    [[nodiscard]] constexpr int rectangle_segment_count(
            int width, int height, bool radius_guide)
    {
        return 2 * (edge_segment_count(width, radius_guide, false) +
                    edge_segment_count(height, radius_guide, true));
    }

    static_assert(rectangle_segment_count(6, 6, false) == 4);
    static_assert(rectangle_segment_count(8, 8, false) == 4);
    static_assert(rectangle_segment_count(10, 10, false) == 4);
    static_assert(rectangle_segment_count(12, 12, false) == 4);
    static_assert(rectangle_segment_count(16, 16, false) == 4);
    static_assert(rectangle_segment_count(64, 16, false) == 6);
    static_assert(rectangle_segment_count(16, 64, false) == 6);
    static_assert(rectangle_segment_count(66, 66, true) == 6);
    static_assert(rectangle_segment_count(100, 100, true) == 10);
    static_assert(rectangle_segment_count(134, 134, true) == 12);
    static_assert(rectangle_segment_count(154, 154, true) == 16);
    static_assert(rectangle_segment_count(66, 66, true) +
                  rectangle_segment_count(100, 100, true) +
                  rectangle_segment_count(134, 134, true) +
                  rectangle_segment_count(154, 154, true) == 44);

    [[nodiscard]] constexpr SegmentLength segment_length(int length)
    {
        switch(length)
        {
        case 6: return SegmentLength::SIX;
        case 8: return SegmentLength::EIGHT;
        case 10: return SegmentLength::TEN;
        case 12: return SegmentLength::TWELVE;
        case 16: return SegmentLength::SIXTEEN;
        case 32: return SegmentLength::THIRTY_TWO;
        default: return SegmentLength::SIXTY_FOUR;
        }
    }

    [[nodiscard]] constexpr GuideStyle guide_style(CollisionDebugBoxType type)
    {
        switch(type)
        {
        case CollisionDebugBoxType::HURTBOX: return GuideStyle::HURT;
        case CollisionDebugBoxType::HITBOX: return GuideStyle::HIT;
        case CollisionDebugBoxType::PUSHBOX: return GuideStyle::PUSH;
        case CollisionDebugBoxType::STATIC_OBSTACLE: return GuideStyle::STATIC_OBSTACLE;
        default: return GuideStyle::HURT;
        }
    }

    [[nodiscard]] constexpr GuideStyle guide_style(CollisionDebugRadiusType type)
    {
        switch(type)
        {
        case CollisionDebugRadiusType::DISCOVERY: return GuideStyle::HURT;
        case CollisionDebugRadiusType::DISENGAGE: return GuideStyle::STATIC_OBSTACLE;
        case CollisionDebugRadiusType::MELEE_COMMIT: return GuideStyle::HIT;
        case CollisionDebugRadiusType::RANGED_COMMIT: return GuideStyle::COMMIT;
        case CollisionDebugRadiusType::FLEE: return GuideStyle::PUSH;
        default: return GuideStyle::HURT;
        }
    }

#define DEBUG_SEGMENT_ITEM(NAME) \
    if(vertical) \
    { \
        switch(length) \
        { \
        case SegmentLength::SIX: return bn::sprite_items::NAME##_vertical_line_6; \
        case SegmentLength::EIGHT: return bn::sprite_items::NAME##_vertical_line_8; \
        case SegmentLength::TEN: return bn::sprite_items::NAME##_vertical_line_10; \
        case SegmentLength::TWELVE: return bn::sprite_items::NAME##_vertical_line_12; \
        case SegmentLength::SIXTEEN: return bn::sprite_items::NAME##_vertical_line_16; \
        default: return bn::sprite_items::NAME##_vertical_line_32; \
        } \
    } \
    switch(length) \
    { \
    case SegmentLength::SIX: return bn::sprite_items::NAME##_line_6; \
    case SegmentLength::EIGHT: return bn::sprite_items::NAME##_line_8; \
    case SegmentLength::TEN: return bn::sprite_items::NAME##_line_10; \
    case SegmentLength::TWELVE: return bn::sprite_items::NAME##_line_12; \
    case SegmentLength::SIXTEEN: return bn::sprite_items::NAME##_line_16; \
    case SegmentLength::THIRTY_TWO: return bn::sprite_items::NAME##_line_32; \
    default: return bn::sprite_items::NAME##_line_64; \
    }

    [[nodiscard]] const bn::sprite_item& segment_item(
            GuideStyle style, bool vertical, SegmentLength length)
    {
        switch(style)
        {
        case GuideStyle::HURT: DEBUG_SEGMENT_ITEM(collision_debug_hurt);
        case GuideStyle::HIT: DEBUG_SEGMENT_ITEM(collision_debug_hit);
        case GuideStyle::PUSH: DEBUG_SEGMENT_ITEM(collision_debug_push);
        case GuideStyle::STATIC_OBSTACLE: DEBUG_SEGMENT_ITEM(collision_debug_static_obstacle);
        case GuideStyle::COMMIT: DEBUG_SEGMENT_ITEM(collision_debug_commit);
        default: DEBUG_SEGMENT_ITEM(collision_debug_hurt);
        }
    }

#undef DEBUG_SEGMENT_ITEM
}

void CollisionDebugOverlay::reset()
{
    _enabled = false;
    _clear();
}

void CollisionDebugOverlay::clear()
{
    _clear();
}

void CollisionDebugOverlay::toggle()
{
    _enabled = ! _enabled;
    if(! _enabled)
    {
        _clear();
    }
}

void CollisionDebugOverlay::update(const CollisionDebugBoxList& boxes, const CollisionDebugRadiusList& radii)
{
    if(! _enabled)
    {
        return;
    }

    int segment_index = 0;
    auto render_rectangle = [&](RasterRectangle rectangle, GuideStyle style, bool radius_guide)
    {
        rectangle = fitted_rectangle(rectangle, radius_guide);
        BN_ASSERT(rectangle.width() > 0 && rectangle.height() > 0,
                  "Collision debug guide is too small");

        auto add_segment = [&](bool vertical, int start, int length, int fixed_coordinate)
        {
            BN_ASSERT(segment_index < max_segments, "Collision debug guide capacity exceeded");
            SegmentLength segment = segment_length(length);
            bn::fixed_point position = vertical ?
                    bn::fixed_point(fixed_coordinate, start + bn::fixed(length) / 2) :
                    bn::fixed_point(start + bn::fixed(length) / 2, fixed_coordinate);
            _set_segment(segment_index++, segment_item(style, vertical, segment), position);
        };

        auto add_edge = [&](bool vertical, int start, int extent, int fixed_coordinate)
        {
            int remaining = extent;
            int current = start;
            while(remaining > 0)
            {
                int length = radius_guide ? largest_radius_segment(remaining, vertical) :
                                            largest_physical_segment(remaining);
                if(length > remaining)
                {
                    break;
                }
                add_segment(vertical, current, length, fixed_coordinate);
                current += length;
                remaining -= length;
            }
        };

        add_edge(false, rectangle.left, rectangle.width(), rectangle.top);
        add_edge(false, rectangle.left, rectangle.width(), rectangle.bottom - 1);
        add_edge(true, rectangle.top, rectangle.height(), rectangle.left);
        add_edge(true, rectangle.top, rectangle.height(), rectangle.right - 1);
    };

    for(int index = 0; index < radii.count(); ++index)
    {
        const CollisionDebugRadius& radius = radii.radii()[index];
        render_rectangle(radius_guide_rectangle(radius), guide_style(radius.type), true);
    }

    for(int index = 0; index < boxes.count(); ++index)
    {
        const CollisionDebugBox& box = boxes.boxes()[index];
        render_rectangle(raster_rectangle(box.box), guide_style(box.type), false);
    }

    for(; segment_index < max_segments; ++segment_index)
    {
        _segments[segment_index].reset();
    }
}

bool CollisionDebugOverlay::enabled() const
{
    return _enabled;
}

void CollisionDebugOverlay::_set_segment(
        int index, const bn::sprite_item& item, const bn::fixed_point& position)
{
    bn::optional<bn::sprite_ptr>& segment = _segments[index];
    if(! segment)
    {
        segment = item.create_sprite(0, 0);
        segment->set_z_order(-3);
    }
    else
    {
        segment->set_item(item);
    }
    segment->set_position(position);
}

void CollisionDebugOverlay::_clear()
{
    for(bn::optional<bn::sprite_ptr>& segment : _segments)
    {
        segment.reset();
    }
}
