#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"

typedef enum ProjectileTarget {
    PROJECTILETARGET_NONE,
    PROJECTILETARGET_NEAREST,
    PROJECTILETARGET_STRONGEST
} projectiletarget_t;

typedef struct ProjectileDef {
    //
    texture_t* tex;
    animation_t* anim;
    animationclip_t* clip;


    // Data
    uint16_t damage;
    bool sprite_rotates;

    projectiletarget_t target;
} projectiledef_t;

bool projectiledef_init(projectiledef_t* def, const char* key);
void projectiledef_destroy(projectiledef_t* def);