#include "combat/melee/swordsman_attack_visual.h"

#include "bn_sprite_item.h"

#include "bn_sprite_items_swordsman_sword_down.h"
#include "bn_sprite_items_swordsman_slash_down.h"
#include "bn_sprite_items_swordsman_slash_down_left.h"
#include "bn_sprite_items_swordsman_slash_left.h"
#include "bn_sprite_items_swordsman_slash_up_left.h"
#include "bn_sprite_items_swordsman_slash_up.h"
#include "bn_sprite_items_swordsman_slash_up_right.h"
#include "bn_sprite_items_swordsman_slash_right.h"
#include "bn_sprite_items_swordsman_slash_down_right.h"

namespace
{
    constexpr int sword_z_order = -1;
    constexpr int slash_z_order = -2;

    [[nodiscard]] const bn::sprite_item& slash_item(Direction direction)
    {
        switch(direction)
        {
        case Direction::DOWN:
            return bn::sprite_items::swordsman_slash_down;
        case Direction::DOWN_LEFT:
            return bn::sprite_items::swordsman_slash_down_left;
        case Direction::LEFT:
            return bn::sprite_items::swordsman_slash_left;
        case Direction::UP_LEFT:
            return bn::sprite_items::swordsman_slash_up_left;
        case Direction::UP:
            return bn::sprite_items::swordsman_slash_up;
        case Direction::UP_RIGHT:
            return bn::sprite_items::swordsman_slash_up_right;
        case Direction::RIGHT:
            return bn::sprite_items::swordsman_slash_right;
        case Direction::DOWN_RIGHT:
            return bn::sprite_items::swordsman_slash_down_right;
        default:
            return bn::sprite_items::swordsman_slash_down;
        }
    }
}

void SwordsmanAttackVisual::play(const AttackContext& context)
{
    _slash_sprite = slash_item(context.direction).create_sprite(context.position, 0);
    _slash_sprite->set_z_order(slash_z_order);

    if(context.direction == Direction::DOWN)
    {
        _sword_sprite = bn::sprite_items::swordsman_sword_down.create_sprite(context.position, 0);
        _sword_sprite->set_z_order(sword_z_order);
    }
    else
    {
        _sword_sprite.reset();
    }

    _direction = context.direction;
    _frame = 0;
    _ticks_in_frame = 0;
}

void SwordsmanAttackVisual::update()
{
    if(! _slash_sprite)
    {
        return;
    }

    ++_ticks_in_frame;

    if(_ticks_in_frame < ticks_per_frame)
    {
        return;
    }

    _ticks_in_frame = 0;
    ++_frame;

    if(_frame == frame_count)
    {
        _sword_sprite.reset();
        _slash_sprite.reset();
        return;
    }

    if(_sword_sprite)
    {
        _sword_sprite->set_item(bn::sprite_items::swordsman_sword_down, _frame);
    }

    _slash_sprite->set_item(slash_item(_direction), _frame);
}

bool SwordsmanAttackVisual::active() const
{
    return bool(_slash_sprite);
}
