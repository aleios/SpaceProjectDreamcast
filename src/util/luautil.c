#include "luautil.h"

static int lua_readonly_newindex(lua_State *L) {
    return luaL_error(L, "attempt to modify read-only enum");
}

void lua_readonly_table(lua_State* L, const char* name) {
    lua_newtable(L);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_readonly_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pushliteral(L, "locked");
    lua_setfield(L, -2, "__metatable");

    lua_setmetatable(L, -2);
    lua_setglobal(L, name);
}

void lua_readonly_enum(lua_State* L, const char* name, const lua_enum_entry_t* entries)
{
    // Create entries table, push new until sentinel
    lua_newtable(L);
    for (auto e = entries; e->name != nullptr; ++e) {
        lua_pushinteger(L, e->val);
        lua_setfield(L, -2, e->name);
    }

    // Setup readonly metatable
    lua_readonly_table(L, name);
}

bool lua_call_event(lua_State* L, int ref_id, int num_args, int num_results) {
    if (ref_id == LUA_NOREF) {
        return true;
    }

    // Grab function from the registry
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref_id);

    // Move function before the arguments and thencall it.
    lua_insert(L, -1 - num_args);
    if (lua_pcall(L, num_args, num_results, 0) != LUA_OK) {
        printf("Lua event error (ref: %d): %s\n", ref_id, lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}