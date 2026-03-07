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