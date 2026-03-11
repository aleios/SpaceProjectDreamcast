#pragma once

void gameover_screen_init();
void gameover_screen_cleanup();

void gameover_screen_enter(void* data);
void gameover_screen_leave();

void gameover_screen_step(float delta_time);

void gameover_screen_render_op();
void gameover_screen_render_tr();