#pragma once

#include "../components/sprite.h"
#include "../components/animation.h"
#include "../components/collider.h"
#include "../defs/collectable_def.h"

typedef struct Collectable {
    sprite_t sprite;
    transform_t transform;
    animationclip_t* clip;
    circlecollider_t collider;

    float lifetime;
    uint32_t sfx;

    int health;
    int lives;
    int weapon;
} collectable_t;

void collectable_init(collectable_t* collectable, collectabledef_t* def);
void collectable_destroy(collectable_t* collectable);

SHZ_FORCE_INLINE void collectable_set_position(collectable_t* collectable, shz_vec2_t pos) {
    collectable->transform.pos = pos;
}