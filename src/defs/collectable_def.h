#pragma once
#include "../components/animation.h"

typedef struct CollectableDef {
    animation_t* anim;
    animationclip_t* clip;

    float lifetime;
    uint32_t sfx;
    float collider_radius;
    float speed;

    int health;
    int lives;
    int weapon;
    int score;
} collectabledef_t;

bool collectabledef_init(collectabledef_t *def, const char* key);
void collectabledef_destroy(collectabledef_t* def);