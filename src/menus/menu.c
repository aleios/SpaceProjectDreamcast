#include "menu.h"
#include "../gamesettings.h"
#include "../util/math.h"
#include "../globals.h"
#include <stdio.h>
#include <kos.h>

static void menu_recalc_positions(menu_t* menu) {

    const auto font = gamesettings_main_font();

    // Calc label widths
    float total_option_height = 0.0f;
    for (int i = 0; i < menu->total; ++i) {
        const auto item = &menu->options[i];
        shz_vec2_t sz = spritefont_str_size(font, item->label);
        if (sz.x > menu->max_label_width) menu->max_label_width = sz.x;
        total_option_height += font->cell_height;
    }

    // Calculate positions
    for (int i = 0; i < menu->total; ++i) {
        const auto item = &menu->options[i];
        item->pos.x = SCREEN_HALF_WIDTH - (menu->max_label_width / 2.0f);
        item->pos.y = SCREEN_HALF_HEIGHT - (total_option_height / 2.0f) + (font->cell_height * i);
    }
}

static void menu_ensure_size(menu_t* menu, int count) {
    menu->total += count;
    if (menu->total >= menu->capacity) {
        menu->capacity = imax32(2, menu->capacity * 2);
        const auto opts = realloc(menu->options, sizeof(menuoption_t) * menu->capacity);
        if (opts) {
            menu->options = opts;
        } else {
            arch_abort();
        }
    }
}

static bool menu_any_enabled_option(menu_t* menu) {
    for (size_t i = 0; i < menu->total; ++i) {
        if (menu->options[i].enabled) return true;
    }
    return false;
}

void menu_select_first_enabled(menu_t* menu) {
    for (size_t i = 0; i < menu->total; ++i) {
        if (menu->options[i].enabled) {
            menu->selected = i;
            return;
        }
    }
    menu->selected = 0; // fallback if everything is disabled I guess
}

static void menu_move_option(menu_t* menu, int direction) {
    if (menu->total == 0)
        return;
    if (!menu_any_enabled_option(menu))
        return;

    size_t start = menu->selected;

    while (1) {
        if (direction > 0) {
            menu->selected = (uint8_t)((menu->selected + 1) % menu->total);
        } else {
            menu->selected = (uint8_t)((menu->selected + menu->total - 1) % menu->total);
        }

        if (menu->options[menu->selected].enabled)
            return;

        if ((size_t)menu->selected == start) {
            return;
        }
    }
}

static void menu_reset(menu_t* menu) {
    menu->options = nullptr;
    menu->total = 0;
    menu->capacity = 0;
    menu->selected = 0;
    menu->delay = 0.0f;
}

void menu_init(menu_t* menu) {
    menu_reset(menu);
}

void menu_destroy(menu_t* menu) {
    free(menu->options);
    menu_reset(menu);
}

int menu_add_option(menu_t* menu, menuoption_t option) {

    menu_ensure_size(menu, 1);

    menu->options[menu->total-1] = option;

    menu_recalc_positions(menu);

    return menu->total-1;
}

static uint32_t last_buttons;
static constexpr float joy_delay = 250.0f;
void menu_step(menu_t* menu, float delta_time) {

    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
        const uint32_t pressed  = state->buttons & ~last_buttons;
        last_buttons = state->buttons;
        static constexpr int joy_threshold = 100;

        if (pressed & CONT_DPAD_UP) {
            menu_move_option(menu, -1);
        }
        if (pressed & CONT_DPAD_DOWN) {
            menu_move_option(menu, 1);
        }

        menu->delay -= delta_time;
        if (state->joyy <= -joy_threshold && menu->delay <= 0.0f) {
            menu_move_option(menu, -1);
            menu->delay = joy_delay;
        }
        if (state->joyy >= joy_threshold && menu->delay <= 0.0f) {
            menu_move_option(menu, 1);
            menu->delay = joy_delay;
        }

        const auto option = &menu->options[menu->selected];
        if (!option->enabled) {
            continue;
        }
        if (pressed & CONT_A) {
            if (option->type == OPTION_ITEM_BUTTON) {
                if (option->button.selected) {
                    option->button.selected();
                }
            }
        }

        const int dpad_dir = (pressed & CONT_DPAD_RIGHT) - (pressed & CONT_DPAD_LEFT);
        const int joy_dir = (state->joyx >= joy_threshold) - (state->joyx <= -joy_threshold);

        if (dpad_dir != 0 || joy_dir != 0) {
            if (option->type == OPTION_ITEM_NUMERIC) {
                const int new_val = iclamp32(dpad_dir + joy_dir, -1, 1) * option->numeric.step;
                option->numeric.value += new_val;
                option->numeric.value = iclamp32(option->numeric.value, option->numeric.min, option->numeric.max);
                if (option->numeric.changed)
                    option->numeric.changed(option->numeric.value);
            } else if (option->type == OPTION_ITEM_BOOLEAN) {
                option->boolean.value = (dpad_dir + joy_dir) > 0;
                if (option->boolean.changed)
                    option->boolean.changed(option->boolean.value);
            }
        }
    MAPLE_FOREACH_END()

    // TODO: Keyboard controls
}

void menu_render(menu_t* menu) {

    const auto font = gamesettings_main_font();
    for (int i = 0; i < menu->total; ++i) {
        const auto option = &menu->options[i];

        // Selection color
        const uint32_t color = (option->enabled) ? ((menu->selected == i) ? 0xFFFF0000 : 0xFFFFFFFF) : 0xFF333333;

        // label
        spritefont_render(font, option->label, option->pos, color);

        // Render option value (if applicable)
        switch (option->type) {
        case OPTION_ITEM_NUMERIC: {
                char val_buf[12];
                snprintf(val_buf, sizeof(val_buf), "%d", option->numeric.value);
                shz_vec2_t val_pos = option->pos;
                val_pos.x += menu->max_label_width + (font->cell_width);
                spritefont_render(font, val_buf, val_pos, color);
                break;
        }
        case OPTION_ITEM_BOOLEAN: {
                shz_vec2_t val_pos = option->pos;
                val_pos.x += menu->max_label_width + (font->cell_width);
                spritefont_render(font, option->boolean.value ? "True" : "False", val_pos, color);
                break;
        }
        default:
            break;
        }
    }
}