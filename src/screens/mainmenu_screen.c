#include "mainmenu_screen.h"
#include "load_screen.h"
#include "play_screen.h"
#include "screens.h"
#include "../globals.h"

#include "../gamesettings.h"
#include "../defs/gamestate.h"
#include "../sound/sound.h"
#include "../util/readutils.h"

#include "../cache/caches.h"

typedef struct MenuOption {
    const char* label;
    void(*selected)();
    shz_vec2_t pos;
} menuoption_t;

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

#ifdef BUILD_EDITOR
void mainmenu_editor_select() {
    printf("SCREEN SET\n");
    screens_set(SCREEN_EDITOR);
}
#endif

uint8_t current_menu_option;
menuoption_t menu_options[] = {
    {
        .label = "Start",
        .selected = mainmenu_start_select,
    },
    {
        .label = "Select Level",
        .selected = mainmenu_level_select,
    },
    {
        .label = "Options",
        .selected = mainmenu_options_select
    },
#ifdef BUILD_EDITOR
    {
        .label = "Editor",
        .selected = mainmenu_editor_select
    }
#endif
};
size_t total_menu_options;

spritefont_t* menu_font;
cont_state_t prev_state;

void mainmenu_next_option() {
    if(current_menu_option < total_menu_options-1)
        current_menu_option++;
}

void mainmenu_prev_option() {
    if(current_menu_option > 0)
        current_menu_option--;
}

void mainmenu_select_option() {
    menu_options[current_menu_option].selected();
}

void mainmenu_screen_init() {

    menu_font = fontcache_get("main_font");

    shz_vec2_t total_size = shz_vec2_init(0, 0);
    total_menu_options = sizeof(menu_options) / sizeof(menu_options[0]);

    // Calculate total size
    for(int i = 0; i < total_menu_options; ++i) {
        shz_vec2_t sz = spritefont_str_size(menu_font, menu_options[i].label);
        if(total_size.x < sz.x) total_size.x = sz.x;
        total_size.y += menu_font->cell_height;
    }

    shz_vec2_t pen;
    pen.x = SCREEN_HALF_WIDTH - (total_size.x / 2.0f);
    pen.y = SCREEN_HALF_HEIGHT - (total_size.y / 2.0f);

    // Cache individual positions
    for(int i = 0; i < total_menu_options; ++i) {
        menu_options[i].pos = pen;
        pen.y += menu_font->cell_height;
    }
}

void mainmenu_screen_cleanup() {
    fontcache_release(menu_font);
}

void mainmenu_screen_enter(void* data) {
    current_menu_option = 0;
    char mus_path[256];
    path_build_cd(mus_path, sizeof(mus_path), "music", "synth_kobra", "adx");
    soundengine_play_mus(mus_path, true);
}

void mainmenu_screen_leave() {
}

void mainmenu_screen_step(float delta_time) {

    // TODO: Helper functions for collecting inputs
    maple_device_t* ctrl_dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(ctrl_dev) {
        cont_state_t* state = maple_dev_status(ctrl_dev);
        uint32_t pressed = state->buttons & ~prev_state.buttons;

        if(pressed & CONT_DPAD_DOWN) {
            mainmenu_next_option();
        }
        if(pressed & CONT_DPAD_UP) {
            mainmenu_prev_option();
        }

        // TODO: Latch last 'states'
        const int threshold = 25;
        if((state->joyy > threshold) && !(prev_state.joyy > threshold)) {
            mainmenu_next_option();
        }
        if((state->joyy < -threshold) && !(prev_state.joyy < -threshold)) {
            mainmenu_prev_option();
        }

        if(pressed & CONT_A) {
            mainmenu_select_option();
        }

        prev_state = *state;
    }

    maple_device_t* kbd_dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
    if(kbd_dev) {
        kbd_state_t* state = maple_dev_status(kbd_dev);

        if(state->key_states[KBD_KEY_W].is_down && !state->key_states[KBD_KEY_W].was_down) {
            mainmenu_prev_option();
        }
        if(state->key_states[KBD_KEY_S].is_down && !state->key_states[KBD_KEY_S].was_down) {
            mainmenu_next_option();
        }
    }
}

void mainmenu_screen_render_op() {

}

void mainmenu_screen_render_tr() {
    for(int i = 0; i < total_menu_options; ++i) {
        spritefont_render(menu_font, menu_options[i].label, menu_options[i].pos, (current_menu_option == i) ? 0xFFFF0000 : 0xFFFFFFFF);
    }
}
