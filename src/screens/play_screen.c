#include "play_screen.h"
#include "load_screen.h"
#include "../renderer/sprite_renderer.h"
#include "../defs/gamestate.h"
#include "screens.h"
#include "../renderer/sprite_font.h"
#include "../sound/sound.h"
#include "../gamesettings.h"
#include "../globals.h"
#include "../renderer/render_util.h"
#include "../starfield/starfield.h"
#include "../ui/ui.h"
#include "../menus/menu.h"

ui_t ui;

typedef enum PlayScreenState {
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSED,
    PLAY_STATE_FADE_OUT
} playscreen_state_t;

static playscreen_state_t play_state;
static float fadeout_timer;
static float fadeout_start;

static menu_t pause_menu;

static void play_screen_fade_out(float duration) {
    fadeout_timer = duration;
    fadeout_start = duration;
    play_state = PLAY_STATE_FADE_OUT;
}

static void pause_option_resume() {
    play_state = PLAY_STATE_PLAYING;
}

static void pause_option_restart() {
    gamestate_restart_level();
    play_state = PLAY_STATE_PLAYING;
}

static void pause_option_quit() {
    screens_set(SCREEN_MAINMENU);
}

void play_screen_init() {
    ui_init(&ui);

    play_state = PLAY_STATE_PLAYING;

    menu_init(&pause_menu);
    menu_add_button(&pause_menu, "Resume", pause_option_resume);
    menu_add_button(&pause_menu, "Restart", pause_option_restart);
    menu_add_button(&pause_menu, "Quit", pause_option_quit);

    menu_select_first_enabled(&pause_menu);
}

void play_screen_cleanup() {
    ui_destroy(&ui);
}

void play_screen_enter(void* data) {
    play_state = PLAY_STATE_PLAYING;
    fadeout_timer = 0.0f;

    gamestate_setup_playfield();
}

void play_screen_leave() {
    soundengine_stop_mus();
    gamestate_destroy();
}

static uint32_t prev_state;

void play_screen_do_play(float delta_time) {

    // Check if dead.
    if (gamestate_get_health() == 0) {
        gamestate_add_lives(-1);
        if(gamestate_get_lives() == 0) {
            screens_set(SCREEN_MAINMENU);
            return;
        }
        // Restart level
        gamestate_reset_stats(STATS_RESET_FLAG_SCORE | STATS_RESET_FLAG_WEAPON);
        gamestate_set_health(gamesettings_max_health());

        if (g_gamestate.is_playlist) {
            gamestate_load_playlist_level(g_gamestate.playlist_index);
        } else {
            gamestate_load_level(g_gamestate.next_level);
        }
        return;
    }

    if (level_finished(gamestate_level())) {
        play_screen_fade_out(1500.0f);
        gamestate_commit_stats();
        // TODO: Play victory theme.
        // Fade out the screen
        // Display 'stage clear' text.
        return;
    }

    level_step(&g_gamestate.level, delta_time);

    starfield_step(gamestate_starfield(), delta_time);

    // Update player
    player_step(gamestate_get_player(), delta_time);

    // Update enemies
    enemypool_step(gamestate_enemy_pool(), delta_time);

    projectilepool_step(gamestate_player_projpool(), delta_time);
    projectilepool_step(gamestate_enemy_projpool(), delta_time);

    collectablepool_step(gamestate_collectable_pool(), delta_time);
}

void play_screen_do_fade_out(float delta_time) {

    fadeout_timer -= delta_time;
    if (fadeout_timer <= 0.0f) {

        int next_level = gamestate_next_level();
        if (next_level >= 0) {
            gamestate_load_playlist_level(next_level);
        } else {
            screens_set(SCREEN_MAINMENU);
        }
    }
}

void play_screen_step(float delta_time) {

    switch (play_state) {
    case PLAY_STATE_PLAYING:
        play_screen_do_play(delta_time);
        break;
    case PLAY_STATE_FADE_OUT:
        play_screen_do_fade_out(delta_time);
        break;
    case PLAY_STATE_PAUSED:
        menu_step(&pause_menu, delta_time);
        break;
    default:
        break;
    }

    maple_device_t* ctrl_dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(ctrl_dev) {
        cont_state_t* state = maple_dev_status(ctrl_dev);
        uint32_t pressed = state->buttons & ~prev_state;
        prev_state = state->buttons;

        if(pressed & CONT_START && play_state != PLAY_STATE_FADE_OUT) {
            playscreen_state_t new_state = (play_state == PLAY_STATE_PLAYING) ? PLAY_STATE_PAUSED : PLAY_STATE_PLAYING;
            play_state = new_state;
            if (new_state == PLAY_STATE_PAUSED) {
                soundengine_pause_mus();
            } else {
                soundengine_resume_mus();
            }
        }
    }
}

void play_screen_render_op() {
    starfield_render(gamestate_starfield());
}

void play_screen_render_tr() {
    collectablepool_render(gamestate_collectable_pool());
    projectilepool_render(gamestate_player_projpool());
    projectilepool_render(gamestate_enemy_projpool());

    sprite_renderer_render();

    // Debug renders
    if (gamesettings_player_collider()) {
        player_render_debug(gamestate_get_player());
    }
    if (gamesettings_enemy_collider()) {
        enemypool_render_debug(gamestate_enemy_pool());
    }
    if (gamesettings_projectile_collider()) {
        projectilepool_render_debug(gamestate_player_projpool());
        projectilepool_render_debug(gamestate_enemy_projpool());
    }

    // State and UI rendering
    if (play_state == PLAY_STATE_PAUSED) {
        render_rect(shz_vec4_init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), PVR_LIST_TR_POLY, 0x66000000);

        menu_render(&pause_menu);

        // TODO: Might add a title to menu itself...
        auto pos = menu_get_position(&pause_menu);
        pos.y -= 24.0f;
        spritefont_render(gamesettings_main_font(), "Paused", pos, 0xFFFFFFFF);
    }

    ui_render(&ui);

    if (play_state == PLAY_STATE_FADE_OUT) {
        int alpha = (int)255.0f - (int)((fadeout_timer / fadeout_start) * 250.0f);
        int color = alpha << 24;
        render_rect(shz_vec4_init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), PVR_LIST_TR_POLY, color);
    }
}