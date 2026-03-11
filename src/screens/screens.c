#include "screens.h"

#include "mainmenu_screen.h"
#include "play_screen.h"
#include "editor_screen.h"
#include "load_screen.h"
#include "options_screen.h"

enum ScreenFlags {
    SCREEN_FLAGS_INIT,
    SCREEN_FLAGS_CLEANUP_ON_LEAVE
};

typedef struct Screen {
    void(*init)();
    void(*cleanup)();

    void(*enter)(void* data);
    void(*leave)();

    void(*step)(float deltaTime);

    void(*render_op)();
    void(*render_tr)();

    int32_t flags;
} screen_t;

static void screens_noop() {}

static screen_t mainmenu_screen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,

    .init = mainmenu_screen_init,
    .cleanup = mainmenu_screen_cleanup,

    .enter = mainmenu_screen_enter,
    .leave = mainmenu_screen_leave,

    .step = mainmenu_screen_step,

    .render_op = mainmenu_screen_render_op,
    .render_tr = mainmenu_screen_render_tr,
};

static screen_t play_screen = {
    .flags = 0,

    .init = play_screen_init,
    .cleanup = play_screen_cleanup,

    .enter = play_screen_enter,
    .leave = play_screen_leave,

    .step = play_screen_step,

    .render_op = play_screen_render_op,
    .render_tr = play_screen_render_tr,
};

static screen_t load_screen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,
    .init = load_screen_init,
    .cleanup = load_screen_cleanup,

    .enter = load_screen_enter,
    .leave = load_screen_leave,

    .step = load_screen_step,

    .render_op = load_screen_render_op,
    .render_tr = load_screen_render_tr
};

static screen_t gameover_screen = {
    .flags = 0,
};

static screen_t options_screen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,

    .init = options_screen_init,
    .cleanup = options_screen_cleanup,

    .enter = options_screen_enter,
    .leave = options_screen_leave,

    .step = options_screen_step,

    .render_op = options_screen_render_op,
    .render_tr = options_screen_render_tr
};

#ifdef BUILD_EDITOR
screen_t g_editorScreen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,
    .init = editor_screen_init,
    .cleanup = editor_screen_cleanup,

    .enter = editor_screen_enter,
    .leave = editor_screen_leave,

    .step = editor_screen_step,

    .render_op = editor_screen_render_op,
    .render_tr = editor_screen_render_tr
};
#endif

static int8_t current_screen_id = -1;
static screen_t* current_screen = nullptr;

static screen_t* screens[] = {
    [SCREEN_MAINMENU] = &mainmenu_screen,
    [SCREEN_PLAY] = &play_screen,
    [SCREEN_LOAD] = &load_screen,
    [SCREEN_GAMEOVER] = &gameover_screen,
    [SCREEN_OPTIONS] = &options_screen,
#ifdef BUILD_EDITOR
    [SCREEN_EDITOR] = &g_editorScreen
#endif
};

void screens_set(screenid_t screen) {
    screens_set_with_data(screen, nullptr);
}

int8_t screens_current_id() {
    return current_screen_id;
}

void screens_set_with_data(screenid_t screen, void* data) {
    if(screen == current_screen_id || screen >= NUM_SCREENS) {
        return;
    }

    if(current_screen) {
        current_screen->leave();

        if(current_screen->flags & SCREEN_FLAGS_CLEANUP_ON_LEAVE) {
            current_screen->cleanup();
        }
    }

    current_screen_id = screen;
    current_screen = screens[screen];

    if(!(current_screen->flags & SCREEN_FLAGS_INIT)) {
        current_screen->init();
        current_screen->flags |= SCREEN_FLAGS_INIT;
    }

    current_screen->enter(data);
}

void screens_step(float delta_time) {
    current_screen->step(delta_time);
}

void screens_render_op() {
    current_screen->render_op();
}

void screens_render_tr() {
    current_screen->render_tr();
}