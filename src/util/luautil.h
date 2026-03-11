#pragma once
#include <lua/lauxlib.h>
#include <lua/lua.h>

inline int lua_optional_ref(lua_State* L, int table_index, const char* field) {
    table_index = lua_absindex(L, table_index);
    lua_getfield(L, table_index, field);
    if (lua_isfunction(L, -1)) {
        return luaL_ref(L, LUA_REGISTRYINDEX);
    }
    lua_pop(L, 1);
    return LUA_NOREF;
}

typedef struct LuaEnumEntry {
    const char* name;
    lua_Integer val;
} lua_enum_entry_t;

void lua_readonly_table(lua_State* L, const char* name);
void lua_readonly_enum(lua_State* L, const char* name, const lua_enum_entry_t* entries);

inline bool luaL_optboolean(lua_State* L, int index, bool default_val) {
    if (lua_isnoneornil(L, index)) {
        return default_val;
    }
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index);
}

bool lua_call_event(lua_State* L, int ref_id, int num_args, int num_results);