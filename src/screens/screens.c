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

void screens_noop() {}

screen_t g_mainMenuScreen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,

    .init = mainmenu_screen_init,
    .cleanup = mainmenu_screen_cleanup,

    .enter = mainmenu_screen_enter,
    .leave = mainmenu_screen_leave,

    .step = mainmenu_screen_step,

    .render_op = mainmenu_screen_render_op,
    .render_tr = mainmenu_screen_render_tr,
};

screen_t g_playScreen = {
    .flags = 0,

    .init = play_screen_init,
    .cleanup = play_screen_cleanup,

    .enter = play_screen_enter,
    .leave = play_screen_leave,

    .step = play_screen_step,

    .render_op = play_screen_render_op,
    .render_tr = play_screen_render_tr,
};

screen_t g_loadScreen = {
    .flags = SCREEN_FLAGS_CLEANUP_ON_LEAVE,
    .init = load_screen_init,
    .cleanup = load_screen_cleanup,

    .enter = load_screen_enter,
    .leave = load_screen_leave,

    .step = load_screen_step,

    .render_op = load_screen_render_op,
    .render_tr = load_screen_render_tr
};

screen_t g_gameoverScreen = {
    .flags = 0,
};

screen_t g_optionsScreen = {
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

int8_t g_currentScreenId = -1;
screen_t* g_currentScreen = nullptr;

screen_t* g_screens[] = {
    [SCREEN_MAINMENU] = &g_mainMenuScreen,
    [SCREEN_PLAY] = &g_playScreen,
    [SCREEN_LOAD] = &g_loadScreen,
    [SCREEN_GAMEOVER] = &g_gameoverScreen,
    [SCREEN_OPTIONS] = &g_optionsScreen,
#ifdef BUILD_EDITOR
    [SCREEN_EDITOR] = &g_editorScreen
#endif
};

void screens_set(screenid_t screen) {
    screens_set_with_data(screen, nullptr);
}

void screens_set_with_data(screenid_t screen, void* data) {
    if(screen == g_currentScreenId || screen >= NUM_SCREENS) {
        return;
    }

    if(g_currentScreen) {
        g_currentScreen->leave();

        if(g_currentScreen->flags & SCREEN_FLAGS_CLEANUP_ON_LEAVE) {
            g_currentScreen->cleanup();
        }
    }

    g_currentScreenId = screen;
    g_currentScreen = g_screens[screen];

    if(!(g_currentScreen->flags & SCREEN_FLAGS_INIT)) {
        g_currentScreen->init();
        g_currentScreen->flags |= SCREEN_FLAGS_INIT;
    }

    g_currentScreen->enter(data);
}

void screens_step(float delta_time) {
    g_currentScreen->step(delta_time);
}

void screens_render_op() {
    g_currentScreen->render_op();
}

void screens_render_tr() {
    g_currentScreen->render_tr();
}