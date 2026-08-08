#include "character/goblin.h"

#include "bn_array.h"
#include "bn_sprite_items_enemy_telegraph.h"
#include "bn_sprite_items_goblin.h"

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
    constexpr int attack_distance = 19;
    constexpr int roam_direction_frames = 42;
    constexpr int telegraph_frames = 54;
    constexpr int active_frames = 8;
    constexpr int recovery_frames = 28;
    constexpr bn::fixed roam_speed = bn::fixed(1) / 4;
    constexpr bn::fixed chase_speed = bn::fixed(1) / 2;
    constexpr bn::fixed diagonal_ratio(0.70710678f);
    constexpr CollisionBody goblin_collision_body = {
        { { 0, 1, 10, 10 } },
        { { 0, 3, 8, 8 } }
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

    [[nodiscard]] Direction direction_from_components(int horizontal, int vertical, Direction fallback)
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
    _active = true;
    _set_telegraph_visible(false);
    apply_movement(_home_position, Direction::DOWN);
    set_visible(true);
}

void Goblin::update(const WorldBox& player_pushbox)
{
    if(! _active)
    {
        return;
    }

    switch(_state)
    {
    case State::ROAM:
        if(within_distance(position(), player_pushbox.center, discovery_distance))
        {
            _state = State::CHASE;
        }
        else
        {
            _update_roam();
        }
        break;
    case State::CHASE:
        _update_chase(player_pushbox);
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
        _update_return();
        break;
    case State::DEAD:
        break;
    default:
        break;
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

    WorldBox hitbox = world_box(position(), attack_hitboxes[int(_attack_direction)].box);
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

void Goblin::_update_chase(const WorldBox& player_pushbox)
{
    if(! within_distance(position(), player_pushbox.center, disengage_distance))
    {
        _state = State::RETURN;
        return;
    }

    if(within_distance(position(), player_pushbox.center, attack_distance))
    {
        _start_attack(player_pushbox.center);
        return;
    }

    _move_toward(player_pushbox.center, chase_speed, &player_pushbox);
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
    if(--_state_timer == 0)
    {
        _state = State::CHASE;
    }
}

void Goblin::_update_return()
{
    if(within_distance(position(), _home_position, 1))
    {
        _state = State::ROAM;
        _state_timer = roam_direction_frames;
        return;
    }

    _move_toward(_home_position, chase_speed, nullptr);
}

void Goblin::_start_attack(const bn::fixed_point& player_position)
{
    _attack_direction = direction_from_components(sign(player_position.x() - position().x()),
                                                   sign(player_position.y() - position().y()), direction());
    apply_movement(position(), _attack_direction);
    _attack_hit_registry.reset();
    _state = State::TELEGRAPH;
    _state_timer = telegraph_frames;
    _set_telegraph_visible(true);
}

void Goblin::_finish_attack()
{
    _attack_hit_registry.reset();
    _state = State::RECOVERY;
    _state_timer = recovery_frames;
}

void Goblin::_die()
{
    _attack_hit_registry.reset();
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
        _telegraph_sprite.reset();
        return;
    }

    bn::fixed telegraph_y = position().y() - 13;
    if(telegraph_y < -70)
    {
        telegraph_y = -70;
    }

    bn::fixed_point telegraph_position(position().x(), telegraph_y);
    if(! _telegraph_sprite)
    {
        _telegraph_sprite = bn::sprite_items::enemy_telegraph.create_sprite(telegraph_position);
        _telegraph_sprite->set_z_order(-2);
    }
    else
    {
        _telegraph_sprite->set_position(telegraph_position);
    }
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
