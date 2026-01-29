#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"

#include "enemy_event.h"

typedef struct EnemyDef {
    animation_t* anim;

    animationclip_t* clip_idle;
    animationclip_t* clip_left;
    animationclip_t* clip_right;

    uint16_t health; // hmmm

    uint8_t total_events;
    event_t event_stack[MAX_EVENTS];
} enemydef_t;

bool enemydef_init(enemydef_t* def, const char* key);
void enemydef_destroy(enemydef_t* def);