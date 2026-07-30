#include "combat/melee/swordsman_slash_effect.h"

#include "bn_sprite_item.h"

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

void SwordsmanSlashEffect::play(const AttackContext& context)
{
    const bn::sprite_item& item = slash_item(context.direction);
    _sprite = item.create_sprite(context.position, 0);
    _direction = context.direction;
    _frame = 0;
    _ticks_in_frame = 0;
}

void SwordsmanSlashEffect::update()
{
    if(! _sprite)
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
        _sprite.reset();
        return;
    }

    _sprite->set_item(slash_item(_direction), _frame);
}

bool SwordsmanSlashEffect::active() const
{
    return bool(_sprite);
}
