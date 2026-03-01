#pragma once

#include <sh4zam/shz_sh4zam.h>

typedef enum MenuOptionType {
    OPTION_ITEM_LABEL,
    OPTION_ITEM_BUTTON,
    OPTION_ITEM_NUMERIC,
    OPTION_ITEM_BOOLEAN
} menuoptiontype_t;

typedef void(*menubtn_cb)();
typedef void(*menunumeric_cb)(int);
typedef void(*menuboolean_cb)(bool);

typedef struct MenuOption {
    menuoptiontype_t type;
    const char* label;
    shz_vec2_t pos;
    bool enabled;
    union {
        struct {
            menubtn_cb selected;
        } button;
        struct {
            int value;
            int min;
            int max;
            int step;
            menunumeric_cb changed;
        } numeric;
        struct {
            bool value;
            menuboolean_cb changed;
        } boolean;
    };
} menuoption_t;