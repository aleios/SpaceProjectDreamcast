#include "direction.h"
#include "../util/luautil.h"
#include "gamestate.h"

void direction_register_lua() {
    const lua_enum_entry_t direction_entries[] = {
        { "None", DIRECTION_NONE },
        { "Left", DIRECTION_LEFT },
        { "Right", DIRECTION_RIGHT },
        { "Up", DIRECTION_UP },
        { "Down", DIRECTION_DOWN },
        { nullptr, 0 } // Sentinel
    };
    lua_readonly_enum(gamestate_lua(), "Direction", direction_entries);
}