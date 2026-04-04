#include "audio_manager.h"
#include <stdio.h>

/* Static sound storage */
static Sound sounds[7] = {0};
static int audio_initialized = 0;

void InitAudioManager(void) {
    if (audio_initialized)
        return;
    
    InitAudioDevice();
    
    /* Load sound files from assets/sounds directory */
    sounds[SOUND_GUNSHOT] = LoadSound("assets/sounds/gunshot.wav");
    sounds[SOUND_TRIGGER] = LoadSound("assets/sounds/trigger.wav");
    sounds[SOUND_LOCK] = LoadSound("assets/sounds/lock.wav");
    sounds[SOUND_BOLT_UP] = LoadSound("assets/sounds/lock.wav");    // placeholder
    sounds[SOUND_BOLT_BACK] = LoadSound("assets/sounds/lock.wav");  // placeholder
    sounds[SOUND_BOLT_DOWN] = LoadSound("assets/sounds/lock.wav");  // placeholder
    sounds[SOUND_CLIP_IN] = LoadSound("assets/sounds/lock.wav");    // placeholder
    
    audio_initialized = 1;
}

void PlayAudioEffect(SoundType type) {
    if (!audio_initialized)
        return;
    
    if (type < 0 || type >= 7)
        return;
    
    if (IsSoundValid(sounds[type])) {
        PlaySound(sounds[type]);
    }
}

void CloseAudioManager(void) {
    if (!audio_initialized)
        return;
    
    for (int i = 0; i < 7; i++) {
        if (IsSoundValid(sounds[i])) {
            UnloadSound(sounds[i]);
        }
    }
    
    CloseAudioDevice();
    audio_initialized = 0;
}
