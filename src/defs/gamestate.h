#pragma once

#include "../gamesettings.h"
#include "../player/player.h"
#include "../projectile/projectile_pool.h"
#include "../level/level.h"
#include "../enemy/enemypool.h"
#include "../starfield/starfield.h"
#include "../collectable/collectablepool.h"
#include "../util/math.h"
#include <lua/lua.h>

typedef struct GameStats {
    int score;
    int lives;
    int health;
    int current_weapon;
} game_stats_t;

static constexpr int NextLevelStrLen = 256;
typedef struct GameState {
    player_t player;

    enemypool_t enemies;
    projectilepool_t enemy_projectile_pool;
    projectilepool_t player_projectile_pool;
    collectablepool_t collectables;

    starfield_t starfield;

    // Stats
    game_stats_t stats;
    game_stats_t prev_stats;

    level_t level;
    int playlist_index;
    bool is_playlist;

    char next_level[NextLevelStrLen];

    lua_State* lua_state;

    bool initialized;
} gamestate_t;

extern gamestate_t g_gamestate;

void gamestate_init();
void gamestate_destroy();

void gamestate_reset();

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

SHZ_FORCE_INLINE lua_State* gamestate_lua() {
    return g_gamestate.lua_state;
}

//
// -- Health --
//
SHZ_FORCE_INLINE void gamestate_set_health(int health) {
    g_gamestate.stats.health = iclamp32(health, 0, gamesettings_max_health());
}

SHZ_FORCE_INLINE void gamestate_add_health(int health) {
    auto stats = &g_gamestate.stats;
    stats->health += health;
    stats->health = iclamp32(stats->health, 0, gamesettings_max_health());
}

SHZ_FORCE_INLINE int gamestate_get_health() {
    return g_gamestate.stats.health;
}

//
// -- Lives --
//
SHZ_FORCE_INLINE void gamestate_set_lives(int lives) {
    g_gamestate.stats.lives = lives;
}

SHZ_FORCE_INLINE void gamestate_add_lives(int lives) {
    auto stats = &g_gamestate.stats;
    stats->lives += lives;
    stats->lives = iclamp32(stats->lives, 0, gamesettings_max_lives());
}

SHZ_FORCE_INLINE int gamestate_get_lives() {
    return g_gamestate.stats.lives;
}

//
// -- Weapon --
//
SHZ_FORCE_INLINE void gamestate_set_weapon(int weapon) {
    player_t* player = gamestate_get_player();
    g_gamestate.stats.current_weapon = iclamp32(weapon, 0, player_get_total_weapons(player)-1);
}

SHZ_FORCE_INLINE void gamestate_add_weapon_power(int power) {
    auto stats = &g_gamestate.stats;
    player_t* player = gamestate_get_player();
    stats->current_weapon += power;
    stats->current_weapon = iclamp32(stats->current_weapon, 0, player_get_total_weapons(player)-1);
}

SHZ_FORCE_INLINE int gamestate_get_weapon() {
    return g_gamestate.stats.current_weapon;
}

//
// -- Score --
//
SHZ_FORCE_INLINE void gamestate_set_score(int score) {
    g_gamestate.stats.score = score;
}

SHZ_FORCE_INLINE void gamestate_add_score(int score) {
    auto stats = &g_gamestate.stats;
    stats->score += score;
    if (stats->score < 0){
        stats->score = 0;
    }
}

SHZ_FORCE_INLINE int gamestate_get_score() {
    return g_gamestate.stats.score;
}

//
// -- Stats --
//
typedef enum {
    STATS_RESET_FLAG_SCORE = 1,
    STATS_RESET_FLAG_LIVES = 2,
    STATS_RESET_FLAG_HEALTH = 4,
    STATS_RESET_FLAG_WEAPON = 8,
    STATS_RESET_FLAG_ALL = STATS_RESET_FLAG_SCORE | STATS_RESET_FLAG_LIVES | STATS_RESET_FLAG_HEALTH | STATS_RESET_FLAG_WEAPON
} stats_reset_flags_t;

SHZ_FORCE_INLINE void gamestate_reset_stats(stats_reset_flags_t flags) {
    if (flags & STATS_RESET_FLAG_SCORE) {
        g_gamestate.stats.score = g_gamestate.prev_stats.score;
    }

    if (flags & STATS_RESET_FLAG_LIVES) {
        g_gamestate.stats.lives = g_gamestate.prev_stats.lives;
    }

    if (flags & STATS_RESET_FLAG_HEALTH) {
        g_gamestate.stats.health = g_gamestate.prev_stats.health;
    }

    if (flags & STATS_RESET_FLAG_WEAPON) {
        g_gamestate.stats.current_weapon = g_gamestate.prev_stats.current_weapon;
    }
}

SHZ_FORCE_INLINE void gamestate_commit_stats() {
    g_gamestate.prev_stats = g_gamestate.stats;
}

//
// -- Level Loading --
//

void gamestate_setup_playfield();
void gamestate_restart_level();

void gamestate_load_level(const char* level_name);
void gamestate_load_playlist_level(int index);

static inline int gamestate_next_level() {
    if (g_gamestate.is_playlist) {
        int next_idx = g_gamestate.playlist_index + 1;
        return next_idx < gamesettings_total_levels() ? next_idx : -1;
    }
    return -1;
}