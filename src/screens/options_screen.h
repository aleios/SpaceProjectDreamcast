#pragma once

void options_screen_init();
void options_screen_cleanup();

void options_screen_enter(void* data);
void options_screen_leave();

void options_screen_step(float delta_time);

void options_screen_render_op();
void options_screen_render_tr();