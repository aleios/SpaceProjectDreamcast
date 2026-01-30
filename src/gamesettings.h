#pragma once
#include "defs/emitter.h"
#include "util/strpool.h"

typedef struct GameOptions {
    uint8_t music_volume;
    uint8_t sfx_volume;
} gameoptions_t;

typedef struct GameSettings {
    int max_lives;
    int max_health;

    strpool_t strpool;
    uint16_t total_levels;
    const char** playlist_levels;

    gameoptions_t options;
} gamesettings_t;

extern gamesettings_t g_gamesettings;

bool gamesettings_load();
bool gamesettings_save();
void gamesettings_destroy();

SHZ_FORCE_INLINE int gamesettings_max_lives() {
    return g_gamesettings.max_lives;
}

SHZ_FORCE_INLINE int gamesettings_max_health() {
    return g_gamesettings.max_health;
}

SHZ_FORCE_INLINE const char* gamesettings_get_level(int index) {
    if (index < 0 || index >= g_gamesettings.total_levels) {
        return nullptr;
    }
    return g_gamesettings.playlist_levels[index];
}

SHZ_FORCE_INLINE int gamesettings_total_levels() {
    return g_gamesettings.total_levels;
}