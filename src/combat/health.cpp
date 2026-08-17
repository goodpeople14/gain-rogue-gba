#include "combat/health.h"

namespace
{
    [[nodiscard]] constexpr bool health_contract_tests()
    {
        Health health(3);
        if(health.max() != 3 || health.current() != 3 || health.is_dead())
        {
            return false;
        }

        health.damage(1);
        if(health.current() != 2 || health.is_dead())
        {
            return false;
        }

        health.damage(5);
        if(health.current() != 0 || ! health.is_dead())
        {
            return false;
        }

        health.reset();
        return health.current() == 3 && ! health.is_dead();
    }

    static_assert(health_contract_tests());
}
