#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "raylib.h"

/* Sound effect identifiers */
typedef enum {
    SOUND_GUNSHOT,
    SOUND_TRIGGER,
    SOUND_LOCK,
    SOUND_BOLT_UP,
    SOUND_BOLT_BACK,
    SOUND_BOLT_DOWN,
    SOUND_CLIP_IN
} SoundType;

/* Initialize audio system and load sounds */
void InitAudioManager(void);

/* Play a sound effect */
void PlayAudioEffect(SoundType type);

/* Cleanup audio resources */
void CloseAudioManager(void);

#endif
