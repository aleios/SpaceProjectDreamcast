#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"

typedef struct ProjectileDef {
    //
    texture_t* tex;
    animation_t* anim;
    animationclip_t* clip;

    // Data
    uint16_t damage;
    bool sprite_rotates;
} projectiledef_t;

bool projectiledef_init(projectiledef_t* def, const char* key);
void projectiledef_destroy(projectiledef_t* def);