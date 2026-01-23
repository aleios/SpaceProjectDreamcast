#include "sound.h"

#include <adx.h>
#include <stdio.h>
#include <string.h>
#include <arch/arch.h>
#include <dc/sound/sfxmgr.h>
#include <sh4zam/shz_scalar.h>

#include "../gamesettings.h"
#include "../cache/resourcecache.h"
#include "../util/readutils.h"

typedef struct SoundEffect {
    char* key;
    uint32_t handle;
} soundeffect_t;

typedef struct SoundEngine {
    float fading_in;
    float fade_in_duration;
    float fading_out;
    float fade_out_duration;
    char current_music_key[256];
    bool should_play;
    bool is_looping;

    int sfx_volume;

    resourcecache_t sfx_cache;
} soundengine_t;

static soundengine_t g_soundEngine;

void soundengine_init() {
    adx_init();

    g_soundEngine.fading_in = 0.0f;
    g_soundEngine.fade_in_duration = 0.0f;
    g_soundEngine.fading_out = 0.0f;
    g_soundEngine.fade_out_duration = 0.0f;
    g_soundEngine.current_music_key[0] = '\0';
    g_soundEngine.should_play = false;

    g_soundEngine.sfx_volume = 128;
}

void soundengine_cleanup() {
    adx_stop();
    adx_shutdown();
    g_soundEngine.current_music_key[0] = '\0';
}

void soundengine_play_mus_ex(const char* key, bool loop, float fade_in_time, float fade_out_time) {
    memcpy(g_soundEngine.current_music_key, key, sizeof(g_soundEngine.current_music_key));

    g_soundEngine.should_play = true;
    g_soundEngine.is_looping = loop;
    g_soundEngine.fading_in = fade_in_time;
    g_soundEngine.fade_in_duration = fade_in_time;

    if (adx_is_playing()) {
        g_soundEngine.fading_out = fade_out_time;
        g_soundEngine.fade_out_duration = fade_out_time;
    } else {
        g_soundEngine.fading_out = 0.0f;
        g_soundEngine.fade_out_duration = 0.0f;
    }
}

void soundengine_play_mus(const char* key, bool loop) {
    soundengine_play_mus_ex(key, loop, 0.0f, 0.0f);
}

void soundengine_stop_mus() {
    g_soundEngine.should_play = false;
    g_soundEngine.is_looping = false;
    g_soundEngine.fading_in = 0.0f;
    g_soundEngine.fade_in_duration = 0.0f;
    g_soundEngine.fading_out = 0.0f;
    g_soundEngine.fade_out_duration = 0.0f;
    adx_stop();
}

void soundengine_pause_mus() {
    adx_pause();
}
void soundengine_resume_mus() {
    adx_play();
}

void soundengine_step(float delta_time) {
    const int vol = SHZ_MIN(g_gamesettings.options.music_volume, 255);

    if (g_soundEngine.fading_out > 0.0f) {
        if (!adx_is_playing()) {
            return;
        }

        g_soundEngine.fading_out -= delta_time;

        if (g_soundEngine.fading_out <= 0.0f) {
            g_soundEngine.fading_out = 0.0f;
            adx_volume(0);
            if (!g_soundEngine.should_play) {
                adx_stop();
            }
        } else {
            float ratio = g_soundEngine.fading_out / g_soundEngine.fade_out_duration;
            if (ratio > 1.0f) ratio = 1.0f;
            if (ratio < 0.0f) ratio = 0.0f;
            adx_volume((int)(ratio * (float)vol));
            //set_target_volume((int)(ratio * 255.0f));
            return; // Continue fading out
        }
    }

    if (g_soundEngine.should_play) {
        adx_stop();
        if (!adx_dec(g_soundEngine.current_music_key, g_soundEngine.is_looping)) {
            g_soundEngine.should_play = false;
            g_soundEngine.fading_in = 0.0f;
            return;
        }
        g_soundEngine.should_play = false;

        if (g_soundEngine.fading_in > 0.0f) {
            adx_volume(0);
        } else {
            adx_volume(vol);
        }
    }

    if (g_soundEngine.fading_in > 0.0f) {
        if (!adx_is_playing()) {
            return;
        }
        g_soundEngine.fading_in -= delta_time;

        if (g_soundEngine.fading_in <= 0.0f) {
            g_soundEngine.fading_in = 0.0f;
            adx_volume(vol);
        } else {
            float ratio = 1.0f - (g_soundEngine.fading_in / g_soundEngine.fade_in_duration);
            if (ratio > 1.0f) ratio = 1.0f;
            if (ratio < 0.0f) ratio = 0.0f;
            adx_volume((int)(ratio * (float)vol));
        }
    }
}

uint32_t soundengine_load_sfx(const char* key) {
    // Check if already loaded
    resourcecache_entry_t* entry = resourcecache_get_or_insert(&g_soundEngine.sfx_cache, key);
    if (entry->data != nullptr) {
        return (uintptr_t)entry->data;
    }

    // Not loaded, try loading.
    char path_buf[256];
    if (!path_build_cd(path_buf, sizeof(path_buf), "sfx", key, "wav")) {
        return SFXHND_INVALID;
    }

    const uint32_t handle = snd_sfx_load(path_buf);
    if (handle == SFXHND_INVALID) {
        return SFXHND_INVALID;
    }

    //|> Type punning. Static analyzer bitches and whines, but I don't care.
    entry->data = (void*)(uintptr_t)handle;

    return handle;
}

uint32_t soundengine_get_sfx(const char* key) {
    void* handle = resourcecache_get(&g_soundEngine.sfx_cache, key);
    if (handle == NULL) {
        return SFXHND_INVALID;
    }
    return (uintptr_t)handle;
}

bool soundengine_play_sfx_ex(uint32_t handle, int volume) {
    return snd_sfx_play(handle, SHZ_MIN(volume, g_gamesettings.options.sfx_volume), 128) >= 0;
}

bool soundengine_play_sfx(uint32_t handle) {
    return snd_sfx_play(handle, g_gamesettings.options.sfx_volume, 128) >= 0;
}