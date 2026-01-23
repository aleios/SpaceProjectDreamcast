#include "play_screen.h"
#include "load_screen.h"
#include "../renderer/sprite_renderer.h"
#include "../defs/gamestate.h"
#include "screens.h"
#include "../renderer/sprite_font.h"
#include "../sound/sound.h"
#include "../gamesettings.h"
#include "../globals.h"
#include "../cache/caches.h"
#include "../renderer/render_util.h"

#include "../starfield/starfield.h"

#include "../ui/ui.h"

spritefont_t* font;
ui_t ui;

typedef enum PlayScreenState {
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSED,
    PLAY_STATE_FADE_OUT
} playscreen_state_t;

static playscreen_state_t play_state;
static float fadeout_timer;
static float fadeout_start;

static void play_screen_fade_out(float duration) {
    fadeout_timer = duration;
    fadeout_start = duration;
    play_state = PLAY_STATE_FADE_OUT;
}

void play_screen_init() {
    //spritefont_init(&font, "main_font", 16, 16);
    font = fontcache_get("main_font");
    ui_init(&ui);

    play_state = PLAY_STATE_PLAYING;
}

void play_screen_cleanup() {
    //spritefont_destroy(&font);
    fontcache_release(font);
    ui_destroy(&ui);
}

void play_screen_enter(void* data) {

    bool continue_playlist = false;
    if (data) {
        playscreen_data_t* screenData = data;
        if (screenData->is_playlist && g_gamestate.is_playlist) {
            continue_playlist = true;
        }
    }

    play_state = PLAY_STATE_PLAYING;
    fadeout_timer = 0.0f;
    gamestate_init();

    if (!continue_playlist) {
        g_gamestate.score = 0;
    }

    if(data) {
        playscreen_data_t* screenData = data;

        g_gamestate.is_playlist = screenData->is_playlist;

        // TODO: All the level loading shit needs to go into `load_screen`.
        if (g_gamestate.is_playlist) {
            if (!continue_playlist) {
                g_gamestate.playlist_index = 0;
            }
            
            if (g_gamesettings.total_levels > 0) {
                const char* level = gamesettings_get_level(g_gamestate.playlist_index);
                if (!gamestate_set_level(level, continue_playlist)) {
                    printf("Failed to load playlist level: %s\n", level);
                    screens_set(SCREEN_MAINMENU);
                    return;
                }
            } else {
                printf("Playlist empty!\n");
                screens_set(SCREEN_MAINMENU);
                return;
            }
        } else {
            printf("Level selected: %s\n", screenData->level);

            if (!gamestate_set_level(screenData->level, false)) {
                printf("Failed to load level: %s\n", screenData->level);
                screens_set(SCREEN_MAINMENU);
                return;
            }
        }
    } else {
        printf("No data. Default level\n");

        g_gamestate.is_playlist = false;
        if (!gamestate_set_level("level1", false)) {
            printf("Failed to load level1\n");
            screens_set(SCREEN_MAINMENU);
            return;
        }
    }

    starfield_init(gamestate_starfield(), 100, 300);
}

void play_screen_leave() {

    soundengine_stop_mus();

    starfield_destroy(gamestate_starfield());
    gamestate_destroy();

    printf("Leaving play screen.\n");
}

static uint32_t prev_state;

void play_screen_do_play(float delta_time) {
    if (level_finished(gamestate_level())) {

        play_screen_fade_out(1500.0f);
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

    g_gamestate.score += 1;
}

void play_screen_do_fade_out(float delta_time) {

    fadeout_timer -= delta_time;
    if (fadeout_timer <= 0.0f) {
        if (g_gamestate.is_playlist) {
            printf("Loading next index\n");
            g_gamestate.playlist_index++;
            if (g_gamestate.playlist_index < g_gamesettings.total_levels) {
                const char* level = gamesettings_get_level(g_gamestate.playlist_index);
                screens_set_with_data(SCREEN_LOAD, &(loadscreen_data_t){
                    .level = level,
                    .is_playlist = true,
                    .playlist_index = g_gamestate.playlist_index
                });
            } else {
                printf("Playlist finished!\n");
                screens_set(SCREEN_MAINMENU);
            }
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

    if (play_state == PLAY_STATE_PAUSED) {
        shz_vec2_t pos = shz_vec2_init(SCREEN_HALF_WIDTH - (16.0f * 3), SCREEN_HALF_HEIGHT);

        render_rect(shz_vec4_init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), PVR_LIST_TR_POLY, 0x66000000);
        spritefont_render(font, "Paused", pos, 0xFFFFFFFF);
    }

    ui_render(&ui);

    if (play_state == PLAY_STATE_FADE_OUT) {
        int alpha = (int)255.0f - (int)((fadeout_timer / fadeout_start) * 250.0f);
        int color = alpha << 24;
        render_rect(shz_vec4_init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), PVR_LIST_TR_POLY, color);
    }
}