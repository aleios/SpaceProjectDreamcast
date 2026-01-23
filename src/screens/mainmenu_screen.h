#pragma once

void mainmenu_screen_init();
void mainmenu_screen_cleanup();

void mainmenu_screen_enter(void* data);
void mainmenu_screen_leave();

void mainmenu_screen_step(float delta_time);

void mainmenu_screen_render_op();
void mainmenu_screen_render_tr();