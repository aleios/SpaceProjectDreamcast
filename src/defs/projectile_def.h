#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"

typedef struct ProjectileDef {
    animation_t* anim;
    animationclip_t* clip;

    // Data
    uint16_t damage;       //< Damage when target hit
    float collider_radius; //< Radius of the collision circle
    bool sprite_rotates;   //< Is the sprite affected by the rotation of the projectile

    uint32_t hit_sound;    //< Sound to play when projectile hits something
} projectiledef_t;

bool projectiledef_init(projectiledef_t* def, const char* key);
void projectiledef_destroy(projectiledef_t* def);