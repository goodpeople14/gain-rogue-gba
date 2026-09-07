#include "audio/audio_system.h"

#include "bn_assert.h"
#include "bn_sound.h"
#include "bn_sound_items.h"

void AudioSystem::play_sfx(SfxId id)
{
    switch(id)
    {
    case SfxId::SWORD_SWING:
        // Audio resource exhaustion must not interrupt gameplay.
        bn::sound::play_optional(bn::sound_items::sword_swing);
        break;

    default:
        BN_ERROR("Invalid SFX id: ", int(id));
        break;
    }
}
