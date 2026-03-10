#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"
#include "../scripting/script.h"

#include "enemy_event.h"

static constexpr int ENEMY_WEAPON_SLOTS = 5;
typedef struct EnemyDef {
    animation_t* anim;

    animationclip_t* clip_idle;
    animationclip_t* clip_left;
    animationclip_t* clip_right;

    uint16_t health;
    float collision_radius;
    int16_t score;

    uint8_t total_events;
    event_t event_stack[MAX_EVENTS];

    script_t* script;

    weaponsetdef_t* weapon_slots[ENEMY_WEAPON_SLOTS];
} enemydef_t;

bool enemydef_init(enemydef_t* def, const char* key);
void enemydef_destroy(enemydef_t* def);