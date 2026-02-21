#pragma once

#include "menuoption.h"

typedef struct Menu {
    menuoption_t* options;
    int total;
    int capacity;

    int selected;

    // Positioning
    float max_label_width;

    float delay;
} menu_t;

void menu_init(menu_t* menu);
void menu_destroy(menu_t* menu);

int menu_add_option(menu_t* menu, menuoption_t option);

// Option constructors
SHZ_FORCE_INLINE int menu_add_button_ex(menu_t* menu, const char* label, menubtn_cb cb, bool enabled) {
    return menu_add_option(menu, (menuoption_t){
        .label = label,
        .type = OPTION_ITEM_BUTTON,
        .button =  {
            .selected = cb
        },
        .enabled = enabled
    });
}

SHZ_FORCE_INLINE int menu_add_button(menu_t* menu, const char* label, menubtn_cb cb) {
    return menu_add_button_ex(menu, label, cb, true);
}

SHZ_FORCE_INLINE int menu_add_numeric_ex(menu_t* menu, const char* label, int min, int max, int step, int initial_value, menunumeric_cb cb, bool enabled) {
    return menu_add_option(menu, (menuoption_t){
        .label = label,
        .type = OPTION_ITEM_NUMERIC,
        .numeric = {
            .min = min,
            .max = max,
            .step = step,
            .value = initial_value,
            .changed = cb
        },
        .enabled = enabled
    });
}

SHZ_FORCE_INLINE int menu_add_numeric(menu_t* menu, const char* label, int min, int max, int step, int initial_value) {
    return menu_add_numeric_ex(menu, label, min, max, step, initial_value, nullptr, true);
}

SHZ_FORCE_INLINE int menu_add_boolean_ex(menu_t* menu, const char* label, bool initial_value, menuboolean_cb cb, bool enabled) {
    return menu_add_option(menu, (menuoption_t){
        .label = label,
        .type = OPTION_ITEM_BOOLEAN,
        .boolean = {
            .value = initial_value,
            .changed = cb
        },
        .enabled = enabled
    });
}

SHZ_FORCE_INLINE int menu_add_boolean(menu_t* menu, const char* label, bool initial_value) {
    return menu_add_boolean_ex(menu, label, initial_value, nullptr, true);
}

void menu_step(menu_t* menu, float delta_time);
void menu_render(menu_t* menu);

SHZ_FORCE_INLINE menuoption_t* menu_get_option(menu_t* menu, int index) {
    return &menu->options[index];
}

void menu_select_first_enabled(menu_t* menu);