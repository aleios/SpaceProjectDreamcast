#ifdef BUILD_EDITOR
#include "editor_screen.h"
#include "../level/level.h"
#include "../renderer/sprite_font.h"
#include "../renderer/render_util.h"
#include "../components/collider.h"

#include "../cache/caches.h"

#include "../globals.h"

#include <stdlib.h>

// NOTE: On save, need to sort by the timepoint in the events.
// Most fucked up file on the planet.

typedef struct Editor {

    // Mouse data
    int mouse_x, mouse_y;
    mouse_state_t prev_mouse_state;
    uint32_t prev_cont_buttons;

    texture_t* tex_editor;

    // Event selection
    int selected_event;
    int hovered_event;

    // Context menu
    int context_menu_selection;
    bool context_menu_open;
    shz_vec2_t context_menu_pos;
    shz_vec2_t context_menu_initial_pos;

    // Level data
    float field_time;
    level_t level;

    // Fonts
    spritefont_t font_ui;
    spritefont_t font_text;

    // Level event tracking
    int level_event_capacity;

    // Save dialog
    char dialog_buffer[256];
    bool save_dialog_open;
} editor_t;
static editor_t editor;

constexpr float field_interval = 5.0f;
const float field_interval_full = 50.0f;
const float mouse_limit_x = SCREEN_WIDTH - 7.0f;
const float mouse_limit_y = SCREEN_HEIGHT - 7.0f;

typedef struct EditorContextMenuOption {
    const char* label;
} editor_contextmenu_opt_t;

editor_contextmenu_opt_t context_menu_opts[] = {
    { .label = "Spawn" },
    { .label = "Music" },
    { .label = "Wait Clear Enemies" },
    { .label = "Wait" },
    { .label = "Starfield Speed" }
};

editor_contextmenu_opt_t context_menu_event_opts[] = {
    { .label = "Edit" },
    { .label = "Delete" }
};
bool context_menu_open = false;

static float editor_get_time_at_y(float y) {
    return (SCREEN_HEIGHT - y) + editor.field_time;
}

static float editor_get_y_at_time(float time) {
    return SCREEN_HEIGHT - (time - editor.field_time);
}

static int sort_events(const void* lhs, const void* rhs) {
    levelevent_timepair_t* p1 = (levelevent_timepair_t*)lhs;
    levelevent_timepair_t* p2 = (levelevent_timepair_t*)rhs;
    return p1->pos.y - p2->pos.y;
}

static void editor_sort_level_events() {
    qsort(editor.level.events, editor.level.total_events, sizeof(levelevent_timepair_t), sort_events);
}

static void editor_add_level_event(levelevent_timepair_t ev) {
    //editor.level
    editor.level.total_events++;
    if(editor.level.total_events >= editor.level_event_capacity) {
        editor.level_event_capacity *= 2;
        editor.level_event_capacity = SHZ_MAX(8, editor.level_event_capacity);
        editor.level.events = realloc(editor.level.events, sizeof(levelevent_timepair_t) * editor.level_event_capacity);

        if(!editor.level.events) {
            arch_abort();
            return;
        }
    }

    editor.level.events[editor.level.total_events-1] = ev;
}

static void editor_remove_level_event(int index) {
    if(index >= editor.level.total_events) {
        return;
    }

    if(index == editor.level.total_events - 1) {
        editor.level.total_events--;
        return;
    }

    // Shift back one place.
    memmove(&editor.level.events[index], &editor.level.events[index+1], sizeof(editor.level.events[0]) * (editor.level.total_events - index - 1));

    editor.level.total_events--;
    editor.hovered_event = -1;
    editor.selected_event = -1;
}

static void editor_add_field_time(float time) {
    editor.field_time = fmaxf(0.0f, editor.field_time + time);
}

static void editor_render_context_menu() {
    if (!editor.context_menu_open) 
        return;

    int count = (editor.hovered_event >= 0) ? 2 : 5;
    editor_contextmenu_opt_t* opts = (editor.hovered_event >= 0) ? context_menu_event_opts : context_menu_opts;

    float width = 200.0f;
    float height = count * 12.0f;
    const float padding = 2.0f;
    render_rect(shz_vec4_init(editor.context_menu_pos.x-padding, editor.context_menu_pos.y-padding, width+padding, height+padding), PVR_LIST_TR_POLY, 0xCC555555);

    for (int i = 0; i < count; ++i) {
        uint32_t color = (editor.context_menu_selection == i) ? 0xFFFFFF00 : 0xFFFFFFFF;
        spritefont_render(&editor.font_text, opts[i].label, 
                            shz_vec2_init(editor.context_menu_pos.x + 4, editor.context_menu_pos.y + (i * 12)), color);
    }
}

static void editor_render_textbox(const char* label, char* buffer, size_t buffer_size, shz_vec2_t pos, bool focused) {
    const float box_width = 200.0f;
    const float box_height = 14.0f;
    const float padding = 2.0f;
    
    // Render label
    spritefont_render(&editor.font_text, label, shz_vec2_init(pos.x, pos.y - 10.0f), 0xFFFFFFFF);

    // Render background box
    uint32_t bg_color = focused ? 0xFF444444 : 0xFF222222;
    uint32_t border_color = focused ? 0xFFFFFF00 : 0xFF888888;
    
    // Border
    render_rect(shz_vec4_init(pos.x - 1, pos.y - 1, box_width + 2, box_height + 2), PVR_LIST_TR_POLY, border_color);
    // Inner box
    render_rect(shz_vec4_init(pos.x, pos.y, box_width, box_height), PVR_LIST_TR_POLY, bg_color);

    // Render string buffer
    spritefont_render(&editor.font_text, buffer, shz_vec2_init(pos.x + 4, pos.y + 3), 0xFFFFFFFF);

    // Simple cursor if focused (blink logic could be added here)
    if (focused) {
        float text_width = strlen(buffer) * 8.0f; // Assuming 8x8 font
        render_rect(shz_vec4_init(pos.x + 4 + text_width, pos.y + 2, 2.0f, 10.0f), PVR_LIST_TR_POLY, 0xFFFFFF00);
    }
}

static void editor_open_context_menu() {
    editor.context_menu_open = true;
    // TODO: Need to actually store the timepoint or it won't work proper on scroll.
    editor.context_menu_initial_pos = shz_vec2_init(editor.mouse_x, editor.mouse_y);
    editor.context_menu_pos = editor.context_menu_initial_pos;

    // TODO: Calc width based on which context menu is opened.
    int count = (editor.hovered_event >= 0) ? 2 : 5;
    float width = 200.0f;
    float height = count * 12.0f;

    if (editor.context_menu_pos.x + width > SCREEN_WIDTH) {
        editor.context_menu_pos.x -= width;
    }
    if (editor.context_menu_pos.y + height > SCREEN_HEIGHT) {
        editor.context_menu_pos.y -= height;
    }
}

static void editor_context_menu_selection() {

    if(editor.context_menu_selection < 0) {
        editor.context_menu_open = false;
        return;
    }

    editor_contextmenu_opt_t* opts = (editor.hovered_event >= 0) ? context_menu_event_opts : context_menu_opts;

    if(editor.hovered_event < 0) {
        levelevent_timepair_t pair = { 0 };
        //pair.event.type
        pair.pos = shz_vec2_init(editor.context_menu_initial_pos.x, editor_get_time_at_y(editor.context_menu_initial_pos.y));

        editor_add_level_event(pair);
    } else {
        editor_remove_level_event(editor.hovered_event);
    }

    editor.context_menu_open = false;
}

static void editor_handle_mouse(mouse_state_t* state) {
    editor.mouse_x += state->dx;
    editor.mouse_y += state->dy;
    const uint32_t pressed = state->buttons &~ editor.prev_mouse_state.buttons;
    const uint32_t released = editor.prev_mouse_state.buttons &~ state->buttons;

    if(editor.save_dialog_open) {
        return;
    }

    if (editor.context_menu_open) {
        const int count = (editor.hovered_event >= 0) ? 2 : 5;
        if (editor.mouse_x >= editor.context_menu_pos.x && editor.mouse_x <= editor.context_menu_pos.x + 120.0f) {
            const int idx = (int)((editor.mouse_y - editor.context_menu_pos.y) / 12.0f);
            if (idx >= 0 && idx < count) {
                editor.context_menu_selection = idx;
            } else {
                editor.context_menu_selection = -1;
            }
        }

        if (pressed & MOUSE_LEFTBUTTON) {
            if (editor.context_menu_selection >= 0) {
                editor_context_menu_selection();
            } else {
                editor.context_menu_open = false;
            }
        }
        editor.prev_mouse_state = *state;
        return;
    }

    editor.hovered_event = -1;
    for (int i = 0; i < editor.level.total_events; ++i) {
        levelevent_timepair_t* ev = &editor.level.events[i];
        float ev_y = editor_get_y_at_time(ev->pos.y);
        
        if (collider_test_point_box(shz_vec2_init(editor.mouse_x, editor.mouse_y), 
                                    shz_vec4_init(ev->pos.x - 8, ev_y - 8, 16, 16))) {
            editor.hovered_event = i;
            break;
        }
    }

    if (pressed & MOUSE_RIGHTBUTTON) {
        editor_open_context_menu();
    }

    if (pressed & MOUSE_LEFTBUTTON) {
        editor.selected_event = editor.hovered_event;
    }

    if (state->buttons & MOUSE_LEFTBUTTON && editor.selected_event >= 0) {
        levelevent_timepair_t* ev = &editor.level.events[editor.selected_event];
        ev->pos.x = editor.mouse_x;
        ev->pos.y = editor_get_time_at_y(editor.mouse_y);
    }

    if (released & MOUSE_LEFTBUTTON) {
        editor.selected_event = -1;
        editor_sort_level_events();
    }

    editor.prev_mouse_state = *state;
}

static void editor_handle_controller(cont_state_t* state, float delta_time) {
    const float joy_speed = 0.2f;
    uint32_t pressed  = state->buttons & ~editor.prev_cont_buttons;
    uint32_t released = editor.prev_cont_buttons & ~state->buttons;

    if(editor.save_dialog_open) {
        return;
    }

    editor.mouse_x += (state->joyx / 127.0f) * joy_speed * delta_time;
    editor.mouse_y += (state->joyy / 127.0f) * joy_speed * delta_time;

    if (editor.context_menu_open) {
        if (pressed & CONT_A) {
            editor_context_menu_selection();
        }
        if (pressed & CONT_B) {
            editor.context_menu_open = false;
        }

        editor.prev_cont_buttons = state->buttons;
        return;
    }

    // Hit testing for events (same as mouse)
    editor.hovered_event = -1;
    for (int i = 0; i < editor.level.total_events; ++i) {
        levelevent_timepair_t* ev = &editor.level.events[i];
        float ev_y = editor_get_y_at_time(ev->pos.y);
        
        if (collider_test_point_box(shz_vec2_init(editor.mouse_x, editor.mouse_y), 
                                    shz_vec4_init(ev->pos.x - 8, ev_y - 8, 16, 16))) {
            editor.hovered_event = i;
            break;
        }
    }

    // B button opens context menu
    if (pressed & CONT_B) {
        editor_open_context_menu();
    }

    // A button selects/drags
    if (pressed & CONT_A) {
        if (editor.selected_event < 0) {
            editor.selected_event = editor.hovered_event;
        }
    } else if(released & CONT_A) {
        editor.selected_event = -1;
        editor_sort_level_events();
    }

    if(state->a && editor.selected_event >= 0) {
        levelevent_timepair_t* ev = &editor.level.events[editor.selected_event];
        ev->pos.x = editor.mouse_x;
        ev->pos.y = editor_get_time_at_y(editor.mouse_y);
    }

    editor.prev_cont_buttons = state->buttons;
}

static void editor_handle_keyboard(maple_device_t* dev, kbd_state_t* state) {

    // Save dialog open? Block other events, poll for textual keys.
    if(editor.save_dialog_open) {
        int k;
        while((k = kbd_queue_pop(dev, true)) != KBD_QUEUE_END) {

            const size_t len = strlen(editor.dialog_buffer);
            if(len + 1 < sizeof(editor.dialog_buffer)-1) {
                editor.dialog_buffer[len] = (char)k;
                editor.dialog_buffer[len+1] = '\0';
            }
            
            //editor.dialog_buffer
        }
        return;
    }

    // Open save dialog upon 'ctrl+s'
    if(state->last_modifiers.lctrl && state->key_states[KBD_KEY_S].is_down) {
        editor.save_dialog_open = true;
    }

    // Move the field time, faster if shift is held.
    float interval = state->last_modifiers.lshift ? field_interval_full : field_interval;
    if (state->key_states[KBD_KEY_W].is_down) {
        editor_add_field_time(interval);
    } else if (state->key_states[KBD_KEY_S].is_down) {
        editor_add_field_time(-interval);
    }
}

static void editor_save_level(const char* filename) {
}

void editor_screen_init() {
    // Load enemies
    spritefont_init(&editor.font_ui, "main_font", 16, 16);
    spritefont_init(&editor.font_text, "font_8x8", 8, 8);

    editor.tex_editor = texcache_get("editor");
}

void editor_screen_cleanup() {
    spritefont_destroy(&editor.font_ui);
    spritefont_destroy(&editor.font_text);
    texcache_release(editor.tex_editor);
}

void editor_screen_enter(void* data) {

    editor.mouse_x = SCREEN_HALF_WIDTH;
    editor.mouse_y = SCREEN_HALF_HEIGHT;
    editor.field_time = 0.0f;

    editor.context_menu_open = false;
    editor.context_menu_pos = shz_vec2_init(0.0f, 0.0f);
    editor.context_menu_selection = -1;

    editor.hovered_event = -1;
    editor.selected_event = -1;
    editor.prev_cont_buttons = 0;

    memset(editor.dialog_buffer, 0, sizeof(editor.dialog_buffer));
    editor.save_dialog_open = false;

    level_init(&editor.level, nullptr);
    editor.level_event_capacity = 0;
}

void editor_screen_leave() {
    level_destroy(&editor.level);
}

void editor_screen_step(float delta_time) {
    maple_device_t* mouse_dev = maple_enum_type(0, MAPLE_FUNC_MOUSE);
    if (mouse_dev) {
        mouse_state_t* state = maple_dev_status(mouse_dev);
        editor_handle_mouse(state);
    }

    maple_device_t* kbd_dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
    if (kbd_dev) {
        kbd_state_t* state = maple_dev_status(kbd_dev);

        editor_handle_keyboard(kbd_dev, state);
    }

    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
        float interval = state->y ? field_interval_full : field_interval;
        
        if (state->dpad_up) {
            editor_add_field_time(interval);
        } else if (state->dpad_down) {
            editor_add_field_time(-interval);
        }

        editor_handle_controller(state, delta_time);

        // editor.mouse_x += (state->joyx / 127.0f) * 0.2f * delta_time;
        // editor.mouse_y += (state->joyy / 127.0f) * 0.2f * delta_time;

        // if(state->b) {
        //     editor.context_menu_open = true;
        //     editor.context_menu_pos = shz_vec2_init(editor.mouse_x, editor.mouse_y);
        // }
    MAPLE_FOREACH_END()

    // Clamp mouse and trigger field time adjustments at edges
    if (editor.mouse_x < 0.0f) editor.mouse_x = 0.0f;
    if (editor.mouse_x > mouse_limit_x) editor.mouse_x = mouse_limit_x;

    if (editor.mouse_y < 0.0f) {
        editor.mouse_y = 0.0f;
        editor_add_field_time(field_interval);
        
        editor.context_menu_open = false;
    } else if (editor.mouse_y > mouse_limit_y) {
        editor.mouse_y = mouse_limit_y;
        editor_add_field_time(-field_interval);

        editor.context_menu_open = false;
    }
}

void editor_screen_render_op() {
}

void editor_screen_render_tr() {
    /*
        Calculating time offset at cursor
            - Bottom of screen = field_time
            - Top of screen    = field_time + (SCREEN_HEIGHT)
            - At mouse = (mouse_y + field_time)
    */
    char str_buf[64];

    // Draw time lines
    const float interval = 250.0f;
    float start_time = editor.field_time;
    float end_time = editor.field_time + SCREEN_HEIGHT;

    float first_interval = ((int)(start_time / interval) + 1) * interval;
    if (editor.field_time == 0.0f) first_interval = 0.0f;

    for (float t = first_interval; t < end_time; t += interval) {
        float y = SCREEN_HEIGHT - (t - editor.field_time);
        printf("Y: %f\n", y);
        render_rect(shz_vec4_init(0.0f, y, SCREEN_WIDTH, 1.0f), PVR_LIST_TR_POLY, 0xFF444444);
        snprintf(str_buf, sizeof(str_buf), "%.1fms", t);
        spritefont_render(&editor.font_text, str_buf, shz_vec2_init(SCREEN_HALF_WIDTH - (8.0f * 5.0f), y - 8.0f), 0xFF888888);
    }

    // Draw events
    for (int i = 0; i < editor.level.total_events; ++i) {
        levelevent_timepair_t* ev = &editor.level.events[i];
        if(ev->pos.y < editor.field_time) {
            continue;
        }

        float y = editor_get_y_at_time(ev->pos.y);

        uint32_t color = (editor.hovered_event == i || editor.selected_event == i) ? 0xFFFFFF00 : 0xFFFF0000;
        render_rect(shz_vec4_init(ev->pos.x - 4, y - 4, 8, 8), PVR_LIST_TR_POLY, color);
    }

    editor_render_context_menu();

    if(editor.save_dialog_open) {
        editor_render_textbox("Name:", editor.dialog_buffer, sizeof(editor.dialog_buffer), shz_vec2_init(320.0f, 240.0f), false);
    }

    // Draw UI Elements
    render_textured_quad(editor.tex_editor, shz_vec4_init(0.0f, 0.0f, 1.0f, 1.0f), shz_vec4_init(editor.mouse_x, editor.mouse_y, 8.0f, 8.0f), 0.0f);

    snprintf(str_buf, sizeof(str_buf), "%.1fms", ((480.0f - editor.mouse_y) + editor.field_time));
    spritefont_render(&editor.font_text, str_buf, shz_vec2_init(editor.mouse_x+8.0f, editor.mouse_y), 0xFFFFFFFF);

    snprintf(str_buf, sizeof(str_buf), "Current time: %.1fms", editor.field_time);
    spritefont_render(&editor.font_ui, str_buf, shz_vec2_init(0.0f, 0.0f), 0xFFFFFFFF);
}
#endif