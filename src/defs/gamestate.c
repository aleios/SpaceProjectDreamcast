#include "gamestate.h"
#include "../globals.h"
#include "../gamesettings.h"
#include "../sound/sound.h"
#include "../util/readutils.h"
#include "../util/luautil.h"
#include "direction.h"
#include <lua/lualib.h>

gamestate_t g_gamestate;

static void register_lua_types(lua_State* L) {

    // Entity types
    player_register_lua(L);
    enemy_register_lua(L);

    // Common types and enums
    direction_register_lua();

    // Constants
    {
        lua_newtable(L);
        lua_pushnumber(L, SCREEN_WIDTH);
        lua_setfield(L, -2, "ScreenWidth");
        lua_pushnumber(L, SCREEN_HEIGHT);
        lua_setfield(L, -2, "ScreenHeight");

        lua_readonly_table(L, "Constants");
    }
}

static int gamestate_score_lua(lua_State* L) {

    // Setter
    if (lua_gettop(L) > 0) {
        int v = (int)luaL_checkinteger(L, 1);
        gamestate_set_score(v);
        return 0;
    }

    lua_pushinteger(L, gamestate_get_score());
    return 1;
}

static int gamestate_lives_lua(lua_State* L) {
    // Setter
    if (lua_gettop(L) > 0) {
        int v = (int)luaL_checkinteger(L, 1);
        gamestate_set_lives(v);
        return 0;
    }

    lua_pushinteger(L, gamestate_get_lives());
    return 1;
}

void gamestate_init() {

    enemypool_init(gamestate_enemy_pool(), 20);
    projectilepool_init(gamestate_player_projpool(), 100, PROJECTILE_POOL_OWNER_PLAYER);
    projectilepool_init(gamestate_enemy_projpool(), 350, PROJECTILE_POOL_OWNER_ENEMY);
    collectablepool_init(gamestate_collectable_pool());

    if (!g_gamestate.lua_state) {
        g_gamestate.lua_state = luaL_newstate();
        auto L = g_gamestate.lua_state;
        luaL_openlibs(L);

        register_lua_types(L);

        // Make player available globally.
        const auto player_udata = (player_t**)lua_newuserdata(L, sizeof(player_t*));
        *player_udata = &g_gamestate.player;
        luaL_setmetatable(L, "PlayerMT");
        lua_setglobal(L, "player");

        // Game state data
        {
            lua_newtable(L);
            lua_pushcfunction(L, gamestate_score_lua);
            lua_setfield(L, -2, "score");
            lua_pushcfunction(L, gamestate_lives_lua);
            lua_setfield(L, -2, "lives");
            lua_readonly_table(L, "Game");
        }
    }
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
    enemypool_reset(gamestate_enemy_pool());
    projectilepool_clear(gamestate_player_projpool());
    projectilepool_clear(gamestate_enemy_projpool());
    collectablepool_clear(gamestate_collectable_pool());
}

void gamestate_reset() {
    g_gamestate.playlist_index = 0;
    g_gamestate.stats = (game_stats_t){
        .score = 0,
        .lives = gamesettings_max_lives(),
        .health =  gamesettings_max_health(),
        .current_weapon = 0
    };
    gamestate_commit_stats();
}

bool gamestate_set_level(const char* level_name, bool keep_stats) {

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

void gamestate_restart_level() {
    gamestate_reset_stats(STATS_RESET_FLAG_ALL);
    gamestate_reset_pools();
    level_restart(&g_gamestate.level);

    char mus_path[256];
    path_build_cd(mus_path, sizeof(mus_path), "music", g_gamestate.level.initial_music, "adx");
    soundengine_play_mus_ex(mus_path, true, 0.0f, 0.0f);

    // Preload all the levels enemy defs.

    // Preload all the levels projectile defs.

    player_set_position(gamestate_get_player(), shz_vec2_init(SCREEN_HALF_WIDTH, SCREEN_HEIGHT - 64.0f));
}