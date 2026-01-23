#pragma once
#include <stdbool.h>

typedef struct LoadScreenData {
    const char* level;
    bool is_playlist;
    int playlist_index;
} loadscreen_data_t;

void load_screen_init();
void load_screen_cleanup();

void load_screen_enter(void* data);
void load_screen_leave();

void load_screen_step(float delta_time);

void load_screen_render_op();
void load_screen_render_tr();