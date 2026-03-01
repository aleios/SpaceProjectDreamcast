#include "options_screen.h"

#include "../globals.h"
#include "screens.h"
#include "../gamesettings.h"
#include "../cache/caches.h"
#include "../util/math.h"

#include "../menus/menu.h"

static menu_t options_menu;
static int music_opt_idx;
static int sfx_opt_idx;
static int player_collider_opt_idx;
static int enemy_collider_opt_idx;
static int projectile_collider_opt_idx;

static void option_save() {
    const auto music_opt = menu_get_option(&options_menu, music_opt_idx);
    const auto sfx_opt = menu_get_option(&options_menu, sfx_opt_idx);

    const auto pc_opt = menu_get_option(&options_menu, player_collider_opt_idx);
    const auto ec_opt = menu_get_option(&options_menu, enemy_collider_opt_idx);
    const auto pj_opt = menu_get_option(&options_menu, projectile_collider_opt_idx);

    g_gamesettings.options.music_volume = music_opt->numeric.value;
    g_gamesettings.options.sfx_volume = sfx_opt->numeric.value;

    g_gamesettings.options.player_collider = pc_opt->boolean.value;
    g_gamesettings.options.enemy_collider = ec_opt->boolean.value;
    g_gamesettings.options.projectile_collider = pj_opt->boolean.value;

    gamesettings_save();

    screens_set(SCREEN_MAINMENU);
}

static void option_back() {
    screens_set(SCREEN_MAINMENU);
}

void options_screen_init() {

    menu_init(&options_menu);

    music_opt_idx = menu_add_numeric(&options_menu, "Music", 0, 255, 1, g_gamesettings.options.music_volume);
    sfx_opt_idx = menu_add_numeric(&options_menu, "SFX", 0, 255, 1, g_gamesettings.options.sfx_volume);

    menu_add_label(&options_menu, "Show Colliders");

    player_collider_opt_idx = menu_add_boolean(&options_menu, "Player", g_gamesettings.options.player_collider);
    enemy_collider_opt_idx = menu_add_boolean(&options_menu, "Enemy", g_gamesettings.options.enemy_collider);
    projectile_collider_opt_idx = menu_add_boolean(&options_menu, "Projectile", g_gamesettings.options.projectile_collider);

    menu_add_button(&options_menu, "Save", option_save);
    menu_add_button(&options_menu, "Back", option_back);
}

void options_screen_cleanup() {
    menu_destroy(&options_menu);
}

void options_screen_enter(void* data) {
    const auto music_opt = menu_get_option(&options_menu, music_opt_idx);
    const auto sfx_opt = menu_get_option(&options_menu, sfx_opt_idx);

    const auto pc_opt = menu_get_option(&options_menu, player_collider_opt_idx);
    const auto ec_opt = menu_get_option(&options_menu, enemy_collider_opt_idx);
    const auto pj_opt = menu_get_option(&options_menu, projectile_collider_opt_idx);

    music_opt->numeric.value = g_gamesettings.options.music_volume;
    sfx_opt->numeric.value = g_gamesettings.options.sfx_volume;

    pc_opt->boolean.value = g_gamesettings.options.player_collider;
    ec_opt->boolean.value = g_gamesettings.options.enemy_collider;
    pj_opt->boolean.value = g_gamesettings.options.projectile_collider;

    menu_select_first_enabled(&options_menu);
}

void options_screen_leave() {
}

void options_screen_step(float delta_time) {
    menu_step(&options_menu, delta_time);
}

void options_screen_render_op() {
}

void options_screen_render_tr() {
    menu_render(&options_menu);
}