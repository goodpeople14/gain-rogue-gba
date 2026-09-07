#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

enum class SfxId
{
    SWORD_SWING
};

class AudioSystem
{
public:
    void play_sfx(SfxId id);
};

#endif
