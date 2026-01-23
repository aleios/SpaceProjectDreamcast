#pragma once
#include <kos.h>

typedef struct PlayScreenData {
    const char* level;
    bool is_playlist;
} playscreen_data_t;

void play_screen_init();
void play_screen_cleanup();

void play_screen_enter(void* data);
void play_screen_leave();

void play_screen_step(float delta_time);

void play_screen_render_op();
void play_screen_render_tr();