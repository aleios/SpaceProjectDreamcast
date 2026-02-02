#include "load_screen.h"
#include "screens.h"
#include "play_screen.h"
#include "../renderer/sprite_font.h"
#include "../cache/caches.h"
#include "../globals.h"
#include "../gamesettings.h"
#include <stdio.h>

static float load_timer;
static loadscreen_data_t load_data;

void load_screen_init() {
}

void load_screen_cleanup() {
}

void load_screen_enter(void* data) {
    if (data) {
        load_data = *(loadscreen_data_t*)data;
    } else {
        load_data.level = "level1";
        load_data.is_playlist = false;
        load_data.playlist_index = 0;
    }

    // Delay for 2s to display title card.
    load_timer = 2000.0f;
}

void load_screen_leave() {

}

void load_screen_step(float delta_time) {
    load_timer -= delta_time;
    if (load_timer <= 0) {
        playscreen_data_t play_data = {
            .level = load_data.level,
            .is_playlist = load_data.is_playlist
        };
        screens_set_with_data(SCREEN_PLAY, &play_data);
    }
}

void load_screen_render_op() {

}

void load_screen_render_tr() {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s", load_data.level);

    spritefont_t* font = gamesettings_main_font();

    shz_vec2_t size = spritefont_str_size(font, buffer);
    shz_vec2_t pos = {
        .x = SCREEN_HALF_WIDTH - (size.x / 2.0f),
        .y = SCREEN_HALF_HEIGHT - (size.y / 2.0f)
    };
    
    spritefont_render(font, buffer, pos, 0xFFFFFFFF);
}