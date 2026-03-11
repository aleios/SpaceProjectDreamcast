#include "enemy.h"
#include "../cache/caches.h"
#include "../util/luautil.h"
#include "../defs/gamestate.h"

/*
 *
 * All the Lua glue functions
 *
 */

static void enemy_push_lua(lua_State* L, enemy_t* enemy) {
    const auto enemy_udata = (enemy_t**)lua_newuserdata(L, sizeof(enemy_t*));
    *enemy_udata = enemy;
    luaL_setmetatable(L, "EnemyMT");
    lua_setfield(L, -2, "enemy");
}

void enemy_script_init(enemy_t* enemy, script_t* script) {

    auto handlers = &enemy->event_sys.handlers;
    handlers->init = LUA_NOREF;
    handlers->on_collide_boundary = LUA_NOREF;
    handlers->on_collide_player = LUA_NOREF;
    handlers->on_damage = LUA_NOREF;
    handlers->on_target_arrive = LUA_NOREF;
    handlers->on_despawn = LUA_NOREF;
    handlers->on_step = LUA_NOREF;

    auto L = gamestate_lua();
    int top = lua_gettop(L);

    // Load script bytecode to stack.
    script_load(script);

    // Create environment for enemy script
    lua_newtable(L);

    // Provide the enemy instance inside the local script environment.
    enemy_push_lua(L, enemy);

    // Load global table into environment
    lua_newtable(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    // Set environment (_ENV)
    if (lua_setupvalue(L, -2, 1) == nullptr) {
        printf("Lua error: could not set environment\n");
        arch_abort();
    }

    lua_getupvalue(L, -1, 1);
    enemy->event_sys.env_index = luaL_ref(L, LUA_REGISTRYINDEX);

    // Execute script and gather script callbacks
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        printf("Lua error: %s\n", lua_tostring(L, -1));
        lua_settop(L, top);
        return;
    }

    // Grab event handlers
    if (!lua_istable(L, -1)) {
        printf("No events for script!\n");
        lua_settop(L, top);
        return;
    }
    lua_getfield(L, -1, "handlers");
    if (!lua_istable(L, -1)) {
        printf("No handlers for script!\n");
        lua_settop(L, top);
        return;
    }

    // Get init func (required)
    handlers->init = lua_optional_ref(L, -1, "init");
    if (handlers->init == LUA_NOREF) {
        lua_settop(L, top);
        return;
    }

    // Get event handlers
    handlers->on_collide_boundary = lua_optional_ref(L, -1, "on_collide_boundary");
    handlers->on_collide_player = lua_optional_ref(L, -1, "on_collide_player");

    handlers->on_damage = lua_optional_ref(L, -1, "on_damage");

    handlers->on_target_arrive = lua_optional_ref(L, -1, "on_target_arrive");
    handlers->on_despawn = lua_optional_ref(L, -1, "on_despawn");

    handlers->on_step = lua_optional_ref(L, -1, "on_step");

    lua_pop(L, 2);

    // Call init
    lua_rawgeti(L, LUA_REGISTRYINDEX, enemy->event_sys.handlers.init);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        printf("Error in on_init: %s\n", err);
        lua_pop(L, 1);
    }
}

void enemy_script_destroy(enemy_t* enemy) {
    auto h = &enemy->event_sys.handlers;
    auto L = gamestate_lua();

    if (h->init != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->init);
        h->init = LUA_NOREF;
    }
    if (h->on_collide_boundary != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_collide_boundary);
        h->on_collide_boundary = LUA_NOREF;
    }
    if (h->on_collide_player != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_collide_player);
        h->on_collide_player = LUA_NOREF;
    }
    if (h->on_damage != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_damage);
        h->on_damage = LUA_NOREF;
    }
    if (h->on_target_arrive != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_target_arrive);
        h->on_target_arrive = LUA_NOREF;
    }
    if (h->on_despawn != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_despawn);
        h->on_despawn = LUA_NOREF;
    }

    if (h->on_step != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, h->on_step);
        h->on_step = LUA_NOREF;
    }

    luaL_unref(L, LUA_REGISTRYINDEX, enemy->event_sys.env_index);
    enemy->event_sys.env_index = LUA_NOREF;
}

static enemy_t* enemy_check(lua_State* L, int idx) {
    const auto u = (enemy_t**)luaL_checkudata(L, idx, "EnemyMT");

    luaL_argcheck(L, u != NULL && *u != NULL, idx, "enemy has been destroyed");

    return *u;
}

static int enemy_health_lua(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    // Setter
    if (lua_gettop(L) >= 2) {
        enemy->health = (int)luaL_checkinteger(L, 2);
        return 0;
    }

    // Getter
    lua_pushinteger(L, enemy->health);
    return 1;
}

static int enemy_position_lua(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    // Setter
    if (lua_gettop(L) >= 2) {
        const auto x = (float)luaL_checknumber(L, 2);
        const auto y = (float)luaL_checknumber(L, 3);

        enemy->transform.pos.x = x;
        enemy->transform.pos.y = y;
        return 0;
    }

    // Getter
    lua_pushnumber(L, enemy->transform.pos.x);
    lua_pushnumber(L, enemy->transform.pos.y);
    return 2;
}

static int enemy_immune_lua(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    if (lua_gettop(L) >= 2) {
        const auto n = (bool)lua_toboolean(L, 2);
        enemy->flags.immune = n;
        return 0;
    }

    lua_pushboolean(L, (bool)enemy->flags.immune);
    return 1;
}

static int enemy_is_dead_lua(lua_State* L) {
    const enemy_t* enemy = enemy_check(L, 1);
    lua_pushboolean(L, (bool)enemy->flags.dead);
    return 1;
}

static int enemy_kill_lua(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);
    enemy_set_dead(enemy);
    return 0;
}

static int enemy_id_lua(lua_State* L) {
    const enemy_t* enemy = enemy_check(L, 1);
    lua_pushinteger(L, enemy->uid);
    return 1;
}

// --
// Movement tasks
// --

// args:
// returns: none
static int move_direction(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    const auto task = &enemy->event_sys.movement_task;

    task->active = true;
    task->type = MOVE_DIRECTIONAL;
    task->dir.angle = SHZ_DEG_TO_RAD((float)luaL_checknumber(L, 2));
    task->speed = (float)luaL_checknumber(L, 3);


    return 0;
}

static int move_to(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    auto tx = (float)luaL_checknumber(L, 2);
    auto ty = (float)luaL_checknumber(L, 3);

    const auto task = &enemy->event_sys.movement_task;
    task->active = true;
    task->type = MOVE_POINT;
    task->speed = 0.1f;
    task->target = shz_vec2_init(tx, ty);

    return 0;
}

// args: angle, speed, period, [amplitude=1.0]
static int move_sine(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    auto angle = (float)luaL_checknumber(L, 2);
    auto speed = (float)luaL_checknumber(L, 3);
    auto period = (float)luaL_checknumber(L, 4);
    auto amplitude = (float)luaL_optnumber(L, 5, 1.0f);

    const auto task = &enemy->event_sys.movement_task;
    task->active = true;
    task->type = MOVE_SINE;
    task->speed = speed;

    const float scale = shz_clampf(period, 0.0f, 1.0f);
    task->sine.phase = 0.0f;
    task->sine.omega = 2.0f * SHZ_F_PI * (scale * 2.0f) / 1000.0f;
    task->sine.amplitude = amplitude;

    const shz_sincos_t sc_angle = shz_sincosf(SHZ_DEG_TO_RAD(angle));
    task->sine.fwd  = shz_vec2_init(sc_angle.cos, sc_angle.sin);
    task->sine.perp = shz_vec2_init(-sc_angle.sin, sc_angle.cos);

    return 0;
}

// args: speed, [continuous=true]
static int move_to_player(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    const auto speed = (float)luaL_checknumber(L, 2);

    const bool continuous = luaL_optboolean(L, 3, true);

    const auto task = &enemy->event_sys.movement_task;
    task->active = true;
    task->type = (continuous) ? MOVE_PLAYER_TARGET : MOVE_PLAYER_INITIAL;
    task->speed = speed;
    task->target = player_get_position(gamestate_get_player());

    return 0;
}

// args: none
// returns: none
static int move_stop(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    const auto task = &enemy->event_sys.movement_task;
    task->active = false;

    return 0;
}

// --
// Weapon slots
// --
static int load_weapon(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);

    const auto slot_id = (int)luaL_checkinteger(L, 2);
    const auto def_name = luaL_checkstring(L, 3);

    if (slot_id < 0 || slot_id >= ENEMY_WEAPON_SLOTS) {
        return 0;
    }

    const auto weapon_slot = &enemy->event_sys.weapons[slot_id];

    const auto def = weaponsetcache_get(def_name);
    weaponset_init(&weapon_slot->weapon, def);
    weapon_slot->valid = true;

    return 0;
}

static int activate_weapon(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);
    const auto slot_id = (int)luaL_checkinteger(L, 2);

    if (slot_id < 0 || slot_id >= ENEMY_WEAPON_SLOTS) {
        // TODO: Warn about slot index wrong.
        return 0;
    }
    const auto weapon_slot = &enemy->event_sys.weapons[slot_id];
    if (weapon_slot->valid) {
        weapon_slot->weapon.firing = true;
    }
    return 0;
}

static int deactivate_weapon(lua_State* L) {
    enemy_t* enemy = enemy_check(L, 1);
    const auto slot_id = (int)luaL_checkinteger(L, 2);

    if (slot_id < 0 || slot_id >= ENEMY_WEAPON_SLOTS) {
        // TODO: Warn about slot index wrong.
        return 0;
    }
    const auto weapon = &enemy->event_sys.weapons[slot_id].weapon;
    weapon->firing = false;
    return 0;
}

void enemy_register_lua(lua_State* L) {
    if (luaL_newmetatable(L, "EnemyMT")) {

        // Properties and Flags
        lua_pushcfunction(L, enemy_health_lua);
        lua_setfield(L, -2, "health");

        lua_pushcfunction(L, enemy_position_lua);
        lua_setfield(L, -2, "pos");

        lua_pushcfunction(L, enemy_immune_lua);
        lua_setfield(L, -2, "immune");

        lua_pushcfunction(L, enemy_is_dead_lua);
        lua_setfield(L, -2, "is_dead");

        lua_pushcfunction(L, enemy_kill_lua);
        lua_setfield(L, -2, "kill");

        lua_pushcfunction(L, enemy_id_lua);
        lua_setfield(L, -2, "id");

        // Movement
        {
            lua_pushcfunction(L, move_direction);
            lua_setfield(L, -2, "move_direction");

            lua_pushcfunction(L, move_to);
            lua_setfield(L, -2, "move_to");

            lua_pushcfunction(L, move_sine);
            lua_setfield(L, -2, "move_sine");

            lua_pushcfunction(L, move_to_player);
            lua_setfield(L, -2, "move_to_player");

            lua_pushcfunction(L, move_stop);
            lua_setfield(L, -2, "move_stop");
        }

        // Projectile
        {
            lua_pushcfunction(L, activate_weapon);
            lua_setfield(L, -2, "activate_weapon");

            lua_pushcfunction(L, deactivate_weapon);
            lua_setfield(L, -2, "deactivate_weapon");
        }

        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);
}