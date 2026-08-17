#ifndef COMBAT_HEALTH_H
#define COMBAT_HEALTH_H

class Health
{
public:
    constexpr explicit Health(int max) :
        _max(max),
        _current(max)
    {
    }

    constexpr void damage(int amount)
    {
        if(amount > 0)
        {
            _current = amount >= _current ? 0 : _current - amount;
        }
    }

    constexpr void reset()
    {
        _current = _max;
    }

    [[nodiscard]] constexpr int current() const
    {
        return _current;
    }

    [[nodiscard]] constexpr int max() const
    {
        return _max;
    }

    [[nodiscard]] constexpr bool is_dead() const
    {
        return _current == 0;
    }

private:
    int _max;
    int _current;
};

#endif
