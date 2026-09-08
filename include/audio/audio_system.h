#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

enum class SfxId
{
    SWORD_SWING,
    ENEMY_HIT,
    PLAYER_HIT,
    ENEMY_ALERT,
    CROSSBOW_FIRE,
    ARROW_LAND,
    STAGE_CLEAR,
    PLAYER_DEATH,
    COUNT
};

enum class SfxEvent
{
    NONE,
    ENEMY_HIT,
    PLAYER_HIT,
    ENEMY_ALERT,
    CROSSBOW_FIRE,
    ARROW_LAND,
    STAGE_CLEAR,
    PLAYER_DEATH
};

[[nodiscard]] constexpr SfxEvent enemy_hit_event(int damage)
{
    return damage > 0 ? SfxEvent::ENEMY_HIT : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent player_hit_event(int damage)
{
    return damage > 0 ? SfxEvent::PLAYER_HIT : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent enemy_alert_event(bool entered_alert)
{
    return entered_alert ? SfxEvent::ENEMY_ALERT : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent crossbow_fire_event(bool projectile_spawned)
{
    return projectile_spawned ? SfxEvent::CROSSBOW_FIRE : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent arrow_land_event(bool projectile_landed)
{
    return projectile_landed ? SfxEvent::ARROW_LAND : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent stage_clear_event(bool entered_cleared)
{
    return entered_cleared ? SfxEvent::STAGE_CLEAR : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxEvent player_death_event(bool entered_dead)
{
    return entered_dead ? SfxEvent::PLAYER_DEATH : SfxEvent::NONE;
}

[[nodiscard]] constexpr SfxId sfx_id_for(SfxEvent event)
{
    switch(event)
    {
    case SfxEvent::ENEMY_HIT:
        return SfxId::ENEMY_HIT;
    case SfxEvent::PLAYER_HIT:
        return SfxId::PLAYER_HIT;
    case SfxEvent::ENEMY_ALERT:
        return SfxId::ENEMY_ALERT;
    case SfxEvent::CROSSBOW_FIRE:
        return SfxId::CROSSBOW_FIRE;
    case SfxEvent::ARROW_LAND:
        return SfxId::ARROW_LAND;
    case SfxEvent::STAGE_CLEAR:
        return SfxId::STAGE_CLEAR;
    case SfxEvent::PLAYER_DEATH:
        return SfxId::PLAYER_DEATH;
    case SfxEvent::NONE:
    default:
        return SfxId::COUNT;
    }
}

class AudioSystem
{
public:
    void play_sfx(SfxId id);
};

#endif
