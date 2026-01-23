#pragma once
#include "components/animation.h"
#include "components/sprite.h"

typedef struct Animator {
    float accumulator;
    int current_frame;
    int direction;
    animationclip_t* clip;
    sprite_t* sprite;
} animator_t;

void animator_reset(animator_t* anim);

void animator_init(animator_t* anim, sprite_t* sprite, animationclip_t* initial_clip);
void animator_destroy(animator_t* anim);

void animator_set_clip(animator_t* anim, animationclip_t* clip);
void animator_step(animator_t* anim, float delta_time);

void animator_refresh(animator_t* anim);