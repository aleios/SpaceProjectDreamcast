#include "script.h"

#include <lua/lauxlib.h>

#include "../defs/gamestate.h"
#include "../util/readutils.h"
#include "../util/membuffer.h"

typedef struct TBuf {
    char* data;
    int current;
    int len;
} tbuf_t;

static int script_writer(lua_State* L, const void* p, size_t sz, void* ud) {
    if (sz == 0)
        return 0;

    const auto buf = (membuffer_t*)ud;
    membuffer_write(buf, p, sz);
    return 0;
}

bool script_init(script_t* script, const char* key) {

    auto L = gamestate_lua();

    // If lua is not available yet, then fail.
    if (!L) {
        return false;
    }

    // Load the script text from file.
    char path[256] = { 0 };
    path_build_rd(path, sizeof(path), "scripts", key, "lua");

    const file_t script_file = fs_open(path, O_RDONLY);
    if (script_file < 0) {
        printf("Could not open file: %s\n", path);
        return false;
    }

    // Get size of script.
    const ssize_t script_total = fs_total(script_file);
    const ssize_t script_size = sizeof(char) * script_total;

    // Allocate buffer for script.
    const auto script_buffer = (char*)malloc(script_size+1);
    if (!script_buffer) {
        printf("Failed to allocate memory for script buffer!\n");
        fs_close(script_file);
        return false;
    }

    int read_bytes = fs_read(script_file, script_buffer, script_size);
    script_buffer[read_bytes] = '\0';
    fs_close(script_file);

    // Load script into Lua.
    int top = lua_gettop(L);
    if (luaL_loadstring(L, script_buffer) != LUA_OK) {
        printf("Loadbuffer fail: %s\n", lua_tostring(L, -1));
        goto fail;
    }

    // Don't need the script text anymore.
    free(script_buffer);

    // Dump bytecode
    membuffer_init(&script->data, 4096);
    if (lua_dump(L, script_writer, &script->data, 1) != LUA_OK) {
        goto fail_luabytecode;
    }

    // Save some memory
    membuffer_shrink(&script->data);

    // Finish up
    lua_pop(L, 1);
    return true;
fail:
    free(script_buffer);
fail_luabytecode:
    lua_settop(L, top);
    membuffer_destroy(&script->data);
    return false;
}

void script_destroy(script_t* script) {
    membuffer_destroy(&script->data);
}

void script_load(script_t* script) {
    const auto len = membuffer_len(&script->data);
    if (len == 0) {
        return;
    }

    auto L = gamestate_lua();
    if (luaL_loadbuffer(L, membuffer_data(&script->data), len, "b") != LUA_OK) {
        printf("Lua error in script_load: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}