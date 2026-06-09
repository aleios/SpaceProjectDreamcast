#include "script.h"

#include <lua/lauxlib.h>

#include "../defs/gamestate.h"
#include "../util/readutils.h"
#include "../util/membuffer.h"

static int script_writer(lua_State* L, const void* p, size_t sz, void* ud) {
    if (sz == 0)
        return 0;

    const auto buf = (membuffer_t*)ud;
    membuffer_write(buf, p, sz);
    return 0;
}

static bool compile_script(script_t* script, lua_State* L, const membuffer_t* script_source) {
    // Load script into Lua.
    int top = lua_gettop(L);
    if (luaL_loadstring(L, script_source->data) != LUA_OK) {
        printf("Loadbuffer fail: %s\n", lua_tostring(L, -1));
        return false;
    }

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
fail_luabytecode:
    lua_settop(L, top);
    membuffer_destroy(&script->data);
    return false;
}

bool script_init_from_memory(script_t* script, membuffer_t* script_buffer) {
    const auto L = gamestate_lua();
    if (!L) {
        return false;
    }

    if (!script_buffer) {
        return false;
    }

    return compile_script(script, L, script_buffer);
}

bool script_init(script_t* script, const char* key) {

    const auto L = gamestate_lua();

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
    membuffer_t script_buffer;
    membuffer_init(&script_buffer, script_size+1);
    if (!script_buffer.data || !script_buffer.capacity) {
        printf("Failed to allocate memory for script buffer!\n");
        fs_close(script_file);
        return false;
    }

    const int read_bytes = fs_read(script_file, script_buffer.data, script_size);
    script_buffer.data[read_bytes] = '\0';
    fs_close(script_file);

    if (!compile_script(script, L, &script_buffer)) {
        membuffer_destroy(&script_buffer);
        return false;
    }

    membuffer_destroy(&script_buffer);
    return true;
}

void script_destroy(script_t* script) {
    membuffer_destroy(&script->data);
}

bool script_load(script_t* script) {
    if (!membuffer_valid(&script->data)) {
        return false;
    }

    const auto len = membuffer_len(&script->data);
    if (len == 0) {
        return false;
    }

    auto L = gamestate_lua();
    if (luaL_loadbuffer(L, membuffer_data(&script->data), len, "b") != LUA_OK) {
        printf("Lua error in script_load: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}