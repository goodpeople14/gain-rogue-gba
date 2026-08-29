#include "enemy/enemy_slot.h"

#include "bn_assert.h"
#include "bn_memory_alias.h"

static_assert(sizeof(Goblin) == 212);
static_assert(sizeof(CrossbowGoblin) == 184);
static_assert(alignof(Goblin) == 4);
static_assert(alignof(CrossbowGoblin) == 4);
static_assert(sizeof(EnemySlot) == 216);
static_assert(alignof(EnemySlot) == 4);

EnemySlot::~EnemySlot()
{
    destroy();
}

bool EnemySlot::occupied() const
{
    return _type != EnemyType::NONE;
}

EnemyType EnemySlot::type() const
{
    return _type;
}

Goblin& EnemySlot::construct_goblin(const bn::fixed_point& position, int target_id)
{
    BN_ASSERT(! occupied(), "Enemy slot already occupied");
    Goblin* result = bn::construct_at(&_storage.goblin, position, target_id);
    _type = EnemyType::GOBLIN;
    return *result;
}

CrossbowGoblin& EnemySlot::construct_crossbow(const bn::fixed_point& position, int target_id)
{
    BN_ASSERT(! occupied(), "Enemy slot already occupied");
    CrossbowGoblin* result = bn::construct_at(&_storage.crossbow, position, target_id);
    _type = EnemyType::CROSSBOW;
    return *result;
}

void EnemySlot::destroy()
{
    switch(_type)
    {
    case EnemyType::GOBLIN:
        bn::destroy_at(&_storage.goblin);
        break;
    case EnemyType::CROSSBOW:
        bn::destroy_at(&_storage.crossbow);
        break;
    case EnemyType::NONE:
        return;
    default:
        BN_ASSERT(false, "Unknown enemy slot type");
        return;
    }

    _type = EnemyType::NONE;
}

Enemy& EnemySlot::enemy()
{
    BN_ASSERT(occupied(), "Enemy slot is empty");
    return _type == EnemyType::GOBLIN ? static_cast<Enemy&>(_storage.goblin) :
                                        static_cast<Enemy&>(_storage.crossbow);
}

const Enemy& EnemySlot::enemy() const
{
    BN_ASSERT(occupied(), "Enemy slot is empty");
    return _type == EnemyType::GOBLIN ? static_cast<const Enemy&>(_storage.goblin) :
                                        static_cast<const Enemy&>(_storage.crossbow);
}

Goblin& EnemySlot::goblin()
{
    BN_ASSERT(_type == EnemyType::GOBLIN, "Enemy slot does not contain a Goblin");
    return _storage.goblin;
}

const Goblin& EnemySlot::goblin() const
{
    BN_ASSERT(_type == EnemyType::GOBLIN, "Enemy slot does not contain a Goblin");
    return _storage.goblin;
}

CrossbowGoblin& EnemySlot::crossbow()
{
    BN_ASSERT(_type == EnemyType::CROSSBOW, "Enemy slot does not contain a CrossbowGoblin");
    return _storage.crossbow;
}

const CrossbowGoblin& EnemySlot::crossbow() const
{
    BN_ASSERT(_type == EnemyType::CROSSBOW, "Enemy slot does not contain a CrossbowGoblin");
    return _storage.crossbow;
}
