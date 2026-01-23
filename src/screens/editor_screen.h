#pragma once
#ifdef BUILD_EDITOR
#include <kos.h>

void editor_screen_init();
void editor_screen_cleanup();

void editor_screen_enter(void* data);
void editor_screen_leave();

void editor_screen_step(float delta_time);

void editor_screen_render_op();
void editor_screen_render_tr();
#endif