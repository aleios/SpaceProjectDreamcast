#include "gamestate.h"
#include "../globals.h"
#include <adx/adx.h>

#include "../gamesettings.h"
#include "../sound/sound.h"
#include "../util/readutils.h"

gamestate_t g_gamestate;

void gamestate_init() {

    enemypool_init(gamestate_enemy_pool(), 20);
    projectilepool_init(gamestate_player_projpool(), 100);
    projectilepool_init(gamestate_enemy_projpool(), 350);
    collectablepool_init(gamestate_collectable_pool());
}

void gamestate_destroy() {
    level_destroy(&g_gamestate.level);
    player_destroy(&g_gamestate.player);

    projectilepool_destroy(gamestate_enemy_projpool());
    projectilepool_destroy(gamestate_player_projpool());
    enemypool_destroy(&g_gamestate.enemies);
    collectablepool_destroy(&g_gamestate.collectables);
}

static void gamestate_reset_pools() {
    enemypool_clear(gamestate_enemy_pool());
    projectilepool_clear(gamestate_player_projpool());
    projectilepool_clear(gamestate_enemy_projpool());
    collectablepool_clear(gamestate_collectable_pool());
}

void gamestate_reset() {
    //gamestate_reset_pools();
    g_gamestate.score = 0;
    g_gamestate.lives = gamesettings_max_lives();
    g_gamestate.health = gamesettings_max_health();
    g_gamestate.playlist_index = 0;
    g_gamestate.weaponset = 0;
}

bool gamestate_set_level(const char* level_name, bool keep_stats) {

    //gamestate_reset_pools();

    // Find level
    if (!level_init(&g_gamestate.level, level_name)) {
        return false;
    }

    char mus_path[256];
    path_build_cd(mus_path, sizeof(mus_path), "music", g_gamestate.level.initial_music, "adx");
    soundengine_play_mus_ex(mus_path, true, 0.0f, 0.0f);

    // Preload all the levels enemy defs.

    // Preload all the levels projectile defs.

    player_init(&g_gamestate.player);
    player_set_position(gamestate_get_player(), shz_vec2_init(SCREEN_HALF_WIDTH, SCREEN_HEIGHT - 64.0f));

    return true;
}