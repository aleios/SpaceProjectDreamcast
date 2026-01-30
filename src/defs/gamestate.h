#pragma once

#include "../player/player.h"
#include "../projectile/projectile_pool.h"
#include "../level/level.h"
#include "../enemy/enemypool.h"
#include "../starfield/starfield.h"
#include "../collectable/collectablepool.h"

typedef struct GameState {
    player_t player;

    enemypool_t enemies;
    projectilepool_t enemy_projectile_pool;
    projectilepool_t player_projectile_pool;
    collectablepool_t collectables;

    level_t level;
    starfield_t starfield;

    // Stats
    int score;
    int lives;
    int health;

    int current_weapon;

    int playlist_index;
    bool is_playlist;
} gamestate_t;

extern gamestate_t g_gamestate;

void gamestate_init();
void gamestate_destroy();

void gamestate_reset();

bool gamestate_set_level(const char* level_name, bool keep_stats);

SHZ_FORCE_INLINE player_t* gamestate_get_player() {
    return &g_gamestate.player;
}

SHZ_FORCE_INLINE projectilepool_t* gamestate_player_projpool() {
    return &g_gamestate.player_projectile_pool;
}

SHZ_FORCE_INLINE projectilepool_t* gamestate_enemy_projpool() {
    return &g_gamestate.enemy_projectile_pool;
}

SHZ_FORCE_INLINE enemypool_t* gamestate_enemy_pool() {
    return &g_gamestate.enemies;
}

SHZ_FORCE_INLINE collectablepool_t* gamestate_collectable_pool() {
    return &g_gamestate.collectables;
}

SHZ_FORCE_INLINE starfield_t* gamestate_starfield() {
    return &g_gamestate.starfield;
}

SHZ_FORCE_INLINE level_t* gamestate_level() {
    return &g_gamestate.level;
}