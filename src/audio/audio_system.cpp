#include "audio/audio_system.h"

#include "bn_assert.h"
#include "bn_sound.h"
#include "bn_sound_items.h"

namespace
{
    static_assert(enemy_hit_event(1) == SfxEvent::ENEMY_HIT);
    static_assert(enemy_hit_event(0) == SfxEvent::NONE);
    static_assert(player_hit_event(1) == SfxEvent::PLAYER_HIT);
    static_assert(player_hit_event(0) == SfxEvent::NONE);
    static_assert(enemy_alert_event(true) == SfxEvent::ENEMY_ALERT);
    static_assert(enemy_alert_event(false) == SfxEvent::NONE);
    static_assert(crossbow_fire_event(true) == SfxEvent::CROSSBOW_FIRE);
    static_assert(crossbow_fire_event(false) == SfxEvent::NONE);
    static_assert(arrow_land_event(true) == SfxEvent::ARROW_LAND);
    static_assert(arrow_land_event(false) == SfxEvent::NONE);
    static_assert(stage_clear_event(true) == SfxEvent::STAGE_CLEAR);
    static_assert(stage_clear_event(false) == SfxEvent::NONE);
    static_assert(player_death_event(true) == SfxEvent::PLAYER_DEATH);
    static_assert(player_death_event(false) == SfxEvent::NONE);
    static_assert(sfx_id_for(SfxEvent::ENEMY_HIT) == SfxId::ENEMY_HIT);
    static_assert(sfx_id_for(SfxEvent::PLAYER_HIT) == SfxId::PLAYER_HIT);
    static_assert(sfx_id_for(SfxEvent::ENEMY_ALERT) == SfxId::ENEMY_ALERT);
    static_assert(sfx_id_for(SfxEvent::CROSSBOW_FIRE) == SfxId::CROSSBOW_FIRE);
    static_assert(sfx_id_for(SfxEvent::ARROW_LAND) == SfxId::ARROW_LAND);
    static_assert(sfx_id_for(SfxEvent::STAGE_CLEAR) == SfxId::STAGE_CLEAR);
    static_assert(sfx_id_for(SfxEvent::PLAYER_DEATH) == SfxId::PLAYER_DEATH);
}

void AudioSystem::play_sfx(SfxId id)
{
    switch(id)
    {
    case SfxId::SWORD_SWING:
        // Audio resource exhaustion must not interrupt gameplay.
        bn::sound::play_optional(bn::sound_items::sword_swing);
        break;

    case SfxId::ENEMY_HIT:
        bn::sound::play_optional(bn::sound_items::enemy_hit);
        break;

    case SfxId::PLAYER_HIT:
        bn::sound::play_optional(bn::sound_items::player_hit);
        break;

    case SfxId::ENEMY_ALERT:
        bn::sound::play_optional(bn::sound_items::enemy_alert);
        break;

    case SfxId::CROSSBOW_FIRE:
        bn::sound::play_optional(bn::sound_items::crossbow_fire);
        break;

    case SfxId::ARROW_LAND:
        bn::sound::play_optional(bn::sound_items::arrow_land);
        break;

    case SfxId::STAGE_CLEAR:
        bn::sound::play_optional(bn::sound_items::stage_clear);
        break;

    case SfxId::PLAYER_DEATH:
        bn::sound::play_optional(bn::sound_items::player_death);
        break;

    default:
        BN_ERROR("Invalid SFX id: ", int(id));
        break;
    }
}
