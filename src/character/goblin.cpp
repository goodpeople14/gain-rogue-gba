#include "character/goblin.h"

#include "bn_array.h"
#include "bn_assert.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin.h"
#include "bn_sprite_items_goblin_awareness_icons.h"
#include "bn_sprite_items_goblin_recovery_hourglass.h"

#include "combat/collision/collision_math.h"
#include "combat/hit_effect_manager.h"
#include "combat/melee/swordsman_attack.h"

namespace
{
    constexpr int goblin_target_id = 10;
    constexpr int player_target_id = 0;
    constexpr int home_radius = 28;
    constexpr int discovery_distance = 42;
    constexpr int disengage_distance = 58;
    constexpr int roam_direction_frames = 42;
    constexpr int telegraph_frames = 54;
    constexpr int active_frames = 8;
    constexpr int recovery_frames = 28;
    constexpr int discovery_flash_frames = 24;
    constexpr int discovery_bulb_white_frames = 10;
    constexpr int return_question_frames = 21;
    constexpr int commit_margin = 2;

    [[nodiscard]] constexpr int discovery_bulb_frame(int remaining_frames)
    {
        return remaining_frames > discovery_flash_frames - discovery_bulb_white_frames ? 0 : 1;
    }

    [[nodiscard]] constexpr int recovery_hourglass_frame(int remaining_frames)
    {
        return remaining_frames > recovery_frames / 2 ? 0 : 1;
    }

    constexpr bn::fixed roam_speed = bn::fixed(1) / 4;
    constexpr bn::fixed chase_speed = bn::fixed(1) / 2;
    constexpr bn::fixed diagonal_ratio(0.70710678f);
    constexpr CollisionBody goblin_collision_body = {
        { { 0, 1, 8, 10 } },
        { { 0, 4, 6, 6 } }
    };
    constexpr bn::array<Direction, 4> roam_directions = {
        Direction::RIGHT, Direction::DOWN, Direction::LEFT, Direction::UP
    };
    [[nodiscard]] constexpr Hitbox hitbox(int x, int y, int width, int height)
    {
        return {{ x, y, width, height }};
    }

    constexpr bn::array<Hitbox, 8> attack_hitboxes = {
        hitbox(0, 14, 12, 10), hitbox(-10, 10, 12, 10),
        hitbox(-14, 0, 10, 12), hitbox(-10, -10, 12, 10),
        hitbox(0, -14, 12, 10), hitbox(10, -10, 12, 10),
        hitbox(14, 0, 10, 12), hitbox(10, 10, 12, 10)
    };

    [[nodiscard]] bool within_distance(const bn::fixed_point& first, const bn::fixed_point& second,
                                       int distance)
    {
        bn::fixed delta_x = first.x() - second.x();
        bn::fixed delta_y = first.y() - second.y();
        return (delta_x * delta_x) + (delta_y * delta_y) <= distance * distance;
    }

    [[nodiscard]] int sign(bn::fixed value)
    {
        return value > 0 ? 1 : value < 0 ? -1 : 0;
    }

    [[nodiscard]] constexpr Direction direction_from_components(int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical)
        {
            return fallback;
        }

        if(vertical > 0)
        {
            return horizontal < 0 ? Direction::DOWN_LEFT : horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN;
        }

        if(vertical < 0)
        {
            return horizontal < 0 ? Direction::UP_LEFT : horizontal > 0 ? Direction::UP_RIGHT : Direction::UP;
        }

        return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
    }

    [[nodiscard]] constexpr int absolute(int value)
    {
        return value < 0 ? -value : value;
    }

    [[nodiscard]] constexpr Direction nearest_direction(int horizontal, int vertical, Direction fallback)
    {
        if(! horizontal && ! vertical)
        {
            return fallback;
        }

        int horizontal_distance = absolute(horizontal);
        int vertical_distance = absolute(vertical);
        if(horizontal_distance * 12 < vertical_distance * 5)
        {
            return vertical < 0 ? Direction::UP : Direction::DOWN;
        }

        if(vertical_distance * 12 < horizontal_distance * 5)
        {
            return horizontal < 0 ? Direction::LEFT : Direction::RIGHT;
        }

        return direction_from_components(horizontal < 0 ? -1 : 1, vertical < 0 ? -1 : 1, fallback);
    }

    [[nodiscard]] Direction nearest_direction(const bn::fixed_point& origin, const bn::fixed_point& target,
                                              Direction fallback)
    {
        return nearest_direction((target.x() - origin.x()).integer(), (target.y() - origin.y()).integer(), fallback);
    }

    void direction_components(Direction direction, int& horizontal, int& vertical)
    {
        static constexpr bn::array<int, 8> horizontal_components = { 0, -1, -1, -1, 0, 1, 1, 1 };
        static constexpr bn::array<int, 8> vertical_components = { 1, 1, 0, -1, -1, -1, 0, 1 };
        horizontal = horizontal_components[int(direction)];
        vertical = vertical_components[int(direction)];
    }

    [[nodiscard]] bn::fixed clamp(bn::fixed value, bn::fixed minimum, bn::fixed maximum)
    {
        return value < minimum ? minimum : value > maximum ? maximum : value;
    }

    [[nodiscard]] constexpr bool attack_direction_tests()
    {
        return nearest_direction(20, 0, Direction::DOWN) == Direction::RIGHT &&
               nearest_direction(-20, 1, Direction::DOWN) == Direction::LEFT &&
               nearest_direction(1, -20, Direction::DOWN) == Direction::UP &&
               nearest_direction(0, 20, Direction::UP) == Direction::DOWN &&
               nearest_direction(20, 9, Direction::DOWN) == Direction::DOWN_RIGHT &&
               nearest_direction(-20, -9, Direction::DOWN) == Direction::UP_LEFT &&
               nearest_direction(20, 8, Direction::DOWN) == Direction::RIGHT &&
               nearest_direction(8, 20, Direction::DOWN) == Direction::DOWN &&
               attack_hitboxes[int(Direction::DOWN)].box.offset_y > 0 &&
               attack_hitboxes[int(Direction::DOWN_LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::DOWN_LEFT)].box.offset_y > 0 &&
               attack_hitboxes[int(Direction::LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::UP_LEFT)].box.offset_x < 0 &&
               attack_hitboxes[int(Direction::UP_LEFT)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::UP)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::UP_RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::UP_RIGHT)].box.offset_y < 0 &&
               attack_hitboxes[int(Direction::RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::DOWN_RIGHT)].box.offset_x > 0 &&
               attack_hitboxes[int(Direction::DOWN_RIGHT)].box.offset_y > 0;
    }

    static_assert(attack_direction_tests());

    [[nodiscard]] WorldBox attack_hitbox(const bn::fixed_point& position, Direction direction)
    {
        return world_box(position, attack_hitboxes[int(direction)].box);
    }

    [[nodiscard]] constexpr WorldBox commit_box(const WorldBox& hitbox)
    {
        return { hitbox.center, hitbox.width - (commit_margin * 2), hitbox.height - (commit_margin * 2) };
    }

    static_assert(commit_box({ { 0, 0 }, 12, 10 }).width == 8);
    static_assert(commit_box({ { 0, 0 }, 12, 10 }).height == 6);
    static_assert(! touches_or_intersects(commit_box({ { 0, 0 }, 12, 10 }), { { 10, 0 }, 10, 10 }));
    static_assert(touches_or_intersects(commit_box({ { 0, 0 }, 12, 10 }), { { 9, 0 }, 10, 10 }));
    static_assert(discovery_bulb_frame(24) == 0);
    static_assert(discovery_bulb_frame(15) == 0);
    static_assert(discovery_bulb_frame(14) == 1);
    static_assert(discovery_bulb_frame(1) == 1);
    static_assert(recovery_hourglass_frame(28) == 0);
    static_assert(recovery_hourglass_frame(15) == 0);
    static_assert(recovery_hourglass_frame(14) == 1);
    static_assert(recovery_hourglass_frame(1) == 1);

    [[nodiscard]] bn::fixed_point status_icon_position(const bn::fixed_point& position)
    {
        bn::fixed icon_y = position.y() - 13;
        if(icon_y < -70)
        {
            icon_y = -70;
        }
        return { position.x(), icon_y };
    }
}

Goblin::Goblin(const bn::fixed_point& home_position) :
    Character(bn::sprite_items::goblin, home_position, Direction::DOWN, chase_speed, goblin_collision_body),
    _home_position(home_position)
{
}

void Goblin::enter()
{
    _state = State::ROAM;
    _state_timer = roam_direction_frames;
    _roam_direction_index = 0;
    _attack_direction = Direction::DOWN;
    _attack_hit_registry.reset();
    _status_icon_frame = 0;
    _status_icon_timer = 0;
    _status_icon = StatusIcon::NONE;
    _active = true;
    _set_telegraph_visible(false);
    apply_movement(_home_position, Direction::DOWN);
    set_visible(true);
}

void Goblin::update(const WorldBox& player_hurtbox, const WorldBox& player_pushbox)
{
    if(! _active)
    {
        return;
    }

    _update_timed_status_icon();

    switch(_state)
    {
    case State::ROAM:
        if(within_distance(position(), player_pushbox.center, discovery_distance))
        {
            _state = State::CHASE;
            _status_icon_timer = discovery_flash_frames;
            _set_awareness_icon(StatusIcon::DISCOVERY_FLASH);
        }
        else
        {
            _update_roam();
        }
        break;
    case State::CHASE:
        _update_chase(player_hurtbox, player_pushbox);
        break;
    case State::TELEGRAPH:
        _update_telegraph();
        break;
    case State::ACTIVE:
        _update_active();
        break;
    case State::RECOVERY:
        _update_recovery();
        break;
    case State::RETURN:
        _update_return(player_pushbox);
        break;
    case State::DEAD:
        break;
    default:
        break;
    }

    if(_status_icon_sprite)
    {
        _status_icon_sprite->set_position(status_icon_position(position()));
    }
}

void Goblin::resolve_player_attack(SwordsmanAttack& attack, HitEffectManager& hit_effects)
{
    if(! _active)
    {
        return;
    }

    int damage = attack.try_hit(goblin_target_id, position(), collision_body().hurtbox);
    if(damage > 0)
    {
        hit_effects.spawn(world_hurtbox().center);
        _die();
    }
}

void Goblin::resolve_player_hit(const bn::fixed_point& player_position, const Hurtbox& player_hurtbox,
                                HitEffectManager& hit_effects)
{
    if(! attack_active() || _attack_hit_registry.contains(player_target_id))
    {
        return;
    }

    WorldBox hitbox = attack_hitbox(position(), _attack_direction);
    WorldBox hurtbox = world_box(player_position, player_hurtbox.box);
    if(touches_or_intersects(hitbox, hurtbox) && _attack_hit_registry.add(player_target_id))
    {
        hit_effects.spawn(hurtbox.center);
    }
}

bool Goblin::active() const
{
    return _active;
}

bool Goblin::attack_active() const
{
    return _active && _state == State::ACTIVE;
}

Goblin::State Goblin::state() const
{
    return _state;
}

WorldBox Goblin::world_hurtbox() const
{
    return world_box(position(), collision_body().hurtbox.box);
}

WorldBoxList<3> Goblin::active_pushboxes() const
{
    WorldBoxList<3> result;
    if(_active)
    {
        result.boxes[0] = world_box(position(), collision_body().pushbox.box);
        result.count = 1;
    }
    return result;
}

void Goblin::append_collision_debug_boxes(const WorldBox& player_hurtbox, CollisionDebugBoxList& boxes) const
{
    if(! _active)
    {
        return;
    }

    boxes.add(world_hurtbox(), CollisionDebugBoxType::HURTBOX);
    boxes.add(world_box(position(), collision_body().pushbox.box), CollisionDebugBoxType::PUSHBOX);
    if(_state == State::CHASE)
    {
        Direction attack_direction = nearest_direction(position(), player_hurtbox.center, direction());
        boxes.add(commit_box(attack_hitbox(position(), attack_direction)), CollisionDebugBoxType::COMMIT_BOX);
    }
    else if(_state == State::TELEGRAPH)
    {
        boxes.add(commit_box(attack_hitbox(position(), _attack_direction)), CollisionDebugBoxType::COMMIT_BOX);
    }
    else if(attack_active())
    {
        boxes.add(attack_hitbox(position(), _attack_direction), CollisionDebugBoxType::HITBOX);
    }
}

void Goblin::_update_roam()
{
    _move_direction(roam_directions[_roam_direction_index], roam_speed, nullptr, true);
    --_state_timer;
    if(_state_timer == 0)
    {
        _roam_direction_index = (_roam_direction_index + 1) % roam_directions.size();
        _state_timer = roam_direction_frames;
    }
}

void Goblin::_update_chase(const WorldBox& player_hurtbox, const WorldBox& player_pushbox)
{
    if(! within_distance(position(), player_hurtbox.center, disengage_distance))
    {
        _state = State::RETURN;
        _status_icon_timer = return_question_frames;
        _set_awareness_icon(StatusIcon::RETURN_QUESTION);
        return;
    }

    Direction attack_direction = nearest_direction(position(), player_hurtbox.center, direction());
    if(touches_or_intersects(commit_box(attack_hitbox(position(), attack_direction)), player_hurtbox))
    {
        _start_attack(attack_direction);
        return;
    }

    _move_toward(player_hurtbox.center, chase_speed, &player_pushbox);
}

void Goblin::_update_telegraph()
{
    _set_telegraph_visible(true);
    if(--_state_timer == 0)
    {
        _set_telegraph_visible(false);
        _state = State::ACTIVE;
        _state_timer = active_frames;
    }
}

void Goblin::_update_active()
{
    if(--_state_timer == 0)
    {
        _finish_attack();
    }
}

void Goblin::_update_recovery()
{
    _set_recovery_hourglass_visible(true);

    if(--_state_timer == 0)
    {
        _set_recovery_hourglass_visible(false);
        _state = State::CHASE;
    }
}

void Goblin::_update_return(const WorldBox& player_pushbox)
{
    if(within_distance(position(), player_pushbox.center, discovery_distance))
    {
        _state = State::CHASE;
        _status_icon_timer = discovery_flash_frames;
        _set_awareness_icon(StatusIcon::DISCOVERY_FLASH);
        return;
    }

    if(within_distance(position(), _home_position, 1))
    {
        _state = State::ROAM;
        _state_timer = roam_direction_frames;
        _status_icon_timer = 0;
        _set_telegraph_visible(false);
        return;
    }

    _move_toward(_home_position, chase_speed, nullptr);
}

void Goblin::_start_attack(Direction direction)
{
    _attack_direction = direction;
    apply_movement(position(), _attack_direction);
    _attack_hit_registry.reset();
    _status_icon_timer = 0;
    _state = State::TELEGRAPH;
    _state_timer = telegraph_frames;
    _set_telegraph_visible(true);
}

void Goblin::_finish_attack()
{
    _attack_hit_registry.reset();
    _state = State::RECOVERY;
    _state_timer = recovery_frames;
    _status_icon_frame = 0;
    _status_icon_timer = 0;
    _set_recovery_hourglass_visible(true);
}

void Goblin::_die()
{
    _attack_hit_registry.reset();
    _status_icon_timer = 0;
    _set_telegraph_visible(false);
    _state = State::DEAD;
    _state_timer = 0;
    _active = false;
    set_visible(false);
}

void Goblin::_set_telegraph_visible(bool visible)
{
    if(! visible)
    {
        _status_icon_sprite.reset();
        _status_icon = StatusIcon::NONE;
        return;
    }

    bn::fixed_point telegraph_position = status_icon_position(position());
    if(! _status_icon_sprite)
    {
        _status_icon_sprite = bn::sprite_items::enemy_telegraph.create_sprite(telegraph_position);
        _status_icon_sprite->set_z_order(-2);
        _status_icon = StatusIcon::TELEGRAPH;
    }
    else
    {
        if(_status_icon != StatusIcon::TELEGRAPH)
        {
            _status_icon_sprite->set_item(bn::sprite_items::enemy_telegraph);
            _status_icon = StatusIcon::TELEGRAPH;
        }
        _status_icon_sprite->set_position(telegraph_position);
    }
}

void Goblin::_set_recovery_hourglass_visible(bool visible)
{
    if(! visible)
    {
        _status_icon_sprite.reset();
        _status_icon = StatusIcon::NONE;
        return;
    }

    int frame = recovery_hourglass_frame(_state_timer);
    bn::fixed_point hourglass_position = status_icon_position(position());
    if(! _status_icon_sprite)
    {
        _status_icon_sprite = bn::sprite_items::goblin_recovery_hourglass.create_sprite(hourglass_position, frame);
        _status_icon_sprite->set_z_order(-2);
        _status_icon = StatusIcon::RECOVERY_HOURGLASS;
        _status_icon_frame = frame;
    }
    else
    {
        if(_status_icon != StatusIcon::RECOVERY_HOURGLASS ||
           _status_icon_frame != frame)
        {
            _status_icon_sprite->set_item(bn::sprite_items::goblin_recovery_hourglass, frame);
            _status_icon = StatusIcon::RECOVERY_HOURGLASS;
            _status_icon_frame = frame;
        }
        _status_icon_sprite->set_position(hourglass_position);
    }
}

void Goblin::_set_awareness_icon(StatusIcon icon)
{
    BN_ASSERT(icon == StatusIcon::DISCOVERY_FLASH || icon == StatusIcon::RETURN_QUESTION);

    int frame = 2;
    if(icon == StatusIcon::DISCOVERY_FLASH)
    {
        frame = discovery_bulb_frame(_status_icon_timer);
    }
    bn::fixed_point icon_position = status_icon_position(position());
    if(! _status_icon_sprite)
    {
        _status_icon_sprite = bn::sprite_items::goblin_awareness_icons.create_sprite(icon_position, frame);
        _status_icon_sprite->set_z_order(-2);
        _status_icon = icon;
        _status_icon_frame = frame;
    }
    else
    {
        if(_status_icon != icon || _status_icon_frame != frame)
        {
            _status_icon_sprite->set_item(bn::sprite_items::goblin_awareness_icons, frame);
            _status_icon = icon;
            _status_icon_frame = frame;
        }
        _status_icon_sprite->set_position(icon_position);
    }
}

void Goblin::_update_timed_status_icon()
{
    if(_status_icon_timer == 0)
    {
        return;
    }

    --_status_icon_timer;
    if(_status_icon_timer == 0)
    {
        _set_telegraph_visible(false);
        return;
    }

    _set_awareness_icon(_status_icon);
}

void Goblin::_move_direction(Direction direction, bn::fixed speed, const WorldBox* blocking_pushbox,
                             bool constrain_to_home)
{
    int horizontal;
    int vertical;
    direction_components(direction, horizontal, vertical);
    bn::fixed step = speed;
    if(horizontal && vertical)
    {
        step *= diagonal_ratio;
    }

    bn::fixed_point next(position().x() + horizontal * step, position().y() + vertical * step);
    if(constrain_to_home)
    {
        next.set_x(clamp(next.x(), _home_position.x() - home_radius, _home_position.x() + home_radius));
        next.set_y(clamp(next.y(), _home_position.y() - home_radius, _home_position.y() + home_radius));
    }

    if(blocking_pushbox)
    {
        WorldBox current = world_box(position(), collision_body().pushbox.box);
        WorldBox candidate = world_box(next, collision_body().pushbox.box);
        if(overlaps_strictly(candidate, *blocking_pushbox) && ! overlaps_strictly(current, *blocking_pushbox))
        {
            apply_movement(position(), direction);
            return;
        }
    }

    apply_movement(next, direction);
}

void Goblin::_move_toward(const bn::fixed_point& target, bn::fixed speed, const WorldBox* blocking_pushbox)
{
    Direction movement_direction = direction_from_components(
            sign(target.x() - position().x()), sign(target.y() - position().y()), direction());
    _move_direction(movement_direction, speed, blocking_pushbox, false);
}
