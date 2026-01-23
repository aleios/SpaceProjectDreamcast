#pragma once

#include "../renderer/texture.h"
#include "../components/animation.h"
#include "../renderer/sprite_font.h"

typedef struct UI {
    texture_t* tex;
    animation_t* anim;
    spritefont_t* font;

    animationframe_t healthbar_frame;
    animationframe_t healthpip_frame;
    animationframe_t life_frame;
} ui_t;

void ui_init(ui_t* ui);
void ui_destroy(ui_t* ui);

void ui_render(ui_t* ui);