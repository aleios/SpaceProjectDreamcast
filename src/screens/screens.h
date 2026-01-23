#pragma once
#include <stdint.h>

typedef enum ScreenID {
    SCREEN_MAINMENU,
    SCREEN_PLAY,
    SCREEN_LOAD,
    SCREEN_GAMEOVER,
    SCREEN_OPTIONS,
#ifdef BUILD_EDITOR
    SCREEN_EDITOR,
#endif
    NUM_SCREENS
} screenid_t;

void screens_set(screenid_t screen);
void screens_set_with_data(screenid_t screen, void* data);

void screens_step(float delta_time);
void screens_render_op();
void screens_render_tr();