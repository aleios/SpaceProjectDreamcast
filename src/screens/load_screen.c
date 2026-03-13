#include "load_screen.h"
#include "screens.h"
#include "play_screen.h"
#include "../renderer/sprite_font.h"
#include "../cache/caches.h"
#include "../globals.h"
#include "../gamesettings.h"
#include <stdio.h>

#include "../defs/gamestate.h"

static float load_timer;
static loadscreen_data_t load_data;

void load_screen_init() {
}

void load_screen_cleanup() {
}

void load_screen_enter(void* data) {
    // Delay for 2s to display title card.
    load_timer = 2000.0f;

    // Initialize level
    gamestate_init();
    if (!level_init(&g_gamestate.level, g_gamestate.next_level)) {
        screens_set(SCREEN_MAINMENU);
    }
}

void load_screen_leave() {

}

void load_screen_step(float delta_time) {
    load_timer -= delta_time;
    if (load_timer <= 0) {
        screens_set(SCREEN_PLAY);
    }
}

void load_screen_render_op() {

}

void load_screen_render_tr() {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s", g_gamestate.next_level);

    spritefont_t* font = gamesettings_main_font();

    shz_vec2_t size = spritefont_str_size(font, buffer);
    shz_vec2_t pos = {
        .x = SCREEN_HALF_WIDTH - (size.x / 2.0f),
        .y = SCREEN_HALF_HEIGHT - (size.y / 2.0f)
    };
    
    spritefont_render(font, buffer, pos, 0xFFFFFFFF);
}