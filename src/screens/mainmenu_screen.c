#include "mainmenu_screen.h"
#include "load_screen.h"
#include "screens.h"

#include "../gamesettings.h"
#include "../defs/gamestate.h"
#include "../sound/sound.h"
#include "../util/readutils.h"

#include "../menus/menu.h"

void mainmenu_start_select() {
    gamestate_reset();
    screens_set_with_data(SCREEN_LOAD, &(loadscreen_data_t){
        .is_playlist = true,
        .level = gamesettings_get_level(0)
    });
}

void mainmenu_level_select() {
    gamestate_reset();
    screens_set_with_data(SCREEN_LOAD, &(loadscreen_data_t){
        .level = "level1",
        .is_playlist = false
    });
}

void mainmenu_options_select() {
    screens_set(SCREEN_OPTIONS);
}

static menu_t main_menu;

void mainmenu_screen_init() {

    menu_init(&main_menu);
    menu_add_button_ex(&main_menu, "Start", mainmenu_start_select, g_gamesettings.total_levels > 0);
    menu_add_button(&main_menu, "Select Level", mainmenu_level_select);
    menu_add_button(&main_menu, "Options", mainmenu_options_select);

    menu_add_numeric_ex(&main_menu, "Test", 0, 100, 1, 0, nullptr, false);
    menu_add_boolean(&main_menu, "Unga", false);
}

void mainmenu_screen_cleanup() {
    menu_destroy(&main_menu);
}

void mainmenu_screen_enter(void* data) {
    char mus_path[256];
    path_build_cd(mus_path, sizeof(mus_path), "music", "synth_kobra", "adx");
    soundengine_play_mus(mus_path, true);

    menu_select_first_enabled(&main_menu);
}

void mainmenu_screen_leave() {
}

void mainmenu_screen_step(float delta_time) {

    menu_step(&main_menu, delta_time);
}

void mainmenu_screen_render_op() {

}

void mainmenu_screen_render_tr() {
    menu_render(&main_menu);
}
