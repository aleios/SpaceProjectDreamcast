#include "options_screen.h"

#include "../globals.h"
#include "screens.h"
#include "../gamesettings.h"
#include "../cache/caches.h"
#include "../util/math.h"

typedef enum OptionItemType {
    OPTION_ITEM_BUTTON,
    OPTION_ITEM_NUMERIC,
    OPTION_ITEM_BOOLEAN
} optionitemtype_t;

typedef enum OptionItemIDs {
    OPTION_ITEM_ID_MUSIC = 0,
    OPTION_ITEM_ID_SFX,
    OPTION_ITEM_ID_SAVE,
    OPTION_ITEM_ID_BACK,
    OPTION_ITEM_ID_COUNT
} optionitemid_t;

typedef struct OptionItem {
    optionitemtype_t type;
    const char* label;
    shz_vec2_t pos;
    union {
        struct {
            void(*selected)();
        } button;
        struct {
            int value;
            int min;
            int max;
            int step;
            void(*changed)(int);
        } numeric;
        struct {
            bool value;
            void(*changed)(bool);
        } boolean;
    };
} optionitem_t;

static void option_save();
static void option_back();

static optionitem_t options[] = {
    {
        OPTION_ITEM_NUMERIC,
        "Music",
        {.x = 0.0f, .y = 0.0f},
        {.numeric = {255, 0, 255, 1}}
    },
    {
        OPTION_ITEM_NUMERIC,
        "SFX",
    {.x = 0.0f, .y = 0.0f},
        {.numeric = {255, 0, 255, 1}}
    },
    {
        OPTION_ITEM_BUTTON,
        "Save",
        {.x = 0.0f, .y = 0.0f},
        {.button = {option_save }}
    },
    {
        OPTION_ITEM_BUTTON,
        "Back",
        {.x = 0.0f, .y = 0.0f},
        {.button = {option_back }}
    },
};
static int selected_option = 0;
static float max_label_width = 0.0f;
static spritefont_t* font;

void options_screen_init() {
    font = fontcache_get("main_font");

    // Calc label widths
    float total_option_height = 0.0f;
    for (int i = 0; i < OPTION_ITEM_ID_COUNT; ++i) {
        optionitem_t* item = &options[i];
        shz_vec2_t sz = spritefont_str_size(font, item->label);
        if (sz.x > max_label_width) max_label_width = sz.x;
        total_option_height += font->cell_height;
    }

    // Calculate positions
    for (int i = 0; i < OPTION_ITEM_ID_COUNT; ++i) {
        optionitem_t* item = &options[i];
        item->pos.x = SCREEN_HALF_WIDTH - (max_label_width / 2.0f);
        item->pos.y = SCREEN_HALF_HEIGHT - (total_option_height / 2.0f) + (font->cell_height * i);
    }
}

void options_screen_cleanup() {
    fontcache_release(font);
}

void options_screen_enter(void* data) {
    selected_option = 0;
    options[OPTION_ITEM_ID_MUSIC].numeric.value = g_gamesettings.options.music_volume;
    options[OPTION_ITEM_ID_SFX].numeric.value = g_gamesettings.options.sfx_volume;
}

void options_screen_leave() {
}

static uint32_t last_buttons;
void options_screen_step(float delta_time) {

    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
        uint32_t pressed  = state->buttons & ~last_buttons;
        if (pressed & CONT_DPAD_UP) {
            if (selected_option > 0) selected_option--;
        }
        if (pressed & CONT_DPAD_DOWN) {
            if (selected_option < OPTION_ITEM_ID_COUNT - 1)
                selected_option++;
        }
        optionitem_t* option = &options[selected_option];
        if (pressed & CONT_A) {
            if (option->type == OPTION_ITEM_BUTTON) {
                if (option->button.selected) {
                    option->button.selected();
                }
            }
        }
        if (state->dpad_left || state->dpad_right) {
            if (option->type == OPTION_ITEM_NUMERIC) {
                int new_val = (state->dpad_left ? -1 : 1) * option->numeric.step;
                option->numeric.value += new_val;
                option->numeric.value = iclamp32(option->numeric.value, option->numeric.min, option->numeric.max);
                if (option->numeric.changed)
                    option->numeric.changed(option->numeric.value);
            } else if (option->type == OPTION_ITEM_BOOLEAN) {
                option->boolean.value = !option->boolean.value;
                if (option->boolean.changed)
                    option->boolean.changed(option->boolean.value);
            }
        }
        last_buttons = state->buttons;
    MAPLE_FOREACH_END()
}

void options_screen_render_op() {

    // Options:
    // * Music volume
    // * SFX volume
}

void options_screen_render_tr() {
    for (int i = 0; i < OPTION_ITEM_ID_COUNT; ++i) {

        // Selection color
        uint32_t color = (i == selected_option) ? 0xFFFF0000 : 0xFFFFFFFF;

        // label
        spritefont_render(font, options[i].label, options[i].pos, color);

        // Render option value (if applicable)
        switch (options[i].type) {
        case OPTION_ITEM_NUMERIC: {
            char val_buf[12];
            snprintf(val_buf, sizeof(val_buf), "%d", options[i].numeric.value);
            shz_vec2_t val_pos = options[i].pos;
            val_pos.x += max_label_width + (font->cell_width * 2.0f);
            spritefont_render(font, val_buf, val_pos, color);
            break;
        }
        case OPTION_ITEM_BOOLEAN: {
            shz_vec2_t val_pos = options[i].pos;
            val_pos.x += max_label_width + (font->cell_width * 2.0f);
            spritefont_render(font, options[i].boolean.value ? "True" : "False", val_pos, color);
            break;
        }
        default:
            break;
        }
    }
}

static void option_save() {
    g_gamesettings.options.music_volume = options[OPTION_ITEM_ID_MUSIC].numeric.value;
    g_gamesettings.options.sfx_volume = options[OPTION_ITEM_ID_SFX].numeric.value;

    gamesettings_save();

    screens_set(SCREEN_MAINMENU);
}

static void option_back() {
    screens_set(SCREEN_MAINMENU);
}