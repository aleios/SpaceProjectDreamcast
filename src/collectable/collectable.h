#pragma once

#include "../components/sprite.h"
#include "../components/animation.h"
#include "../components/collider.h"
#include "../defs/collectable_def.h"
#include "../animator.h"

typedef struct Collectable {
    sprite_t sprite;
    transform_t transform;
    animationclip_t* clip;
    circlecollider_t collider;
    animator_t animator;

    float lifetime;
    float speed;
    uint32_t sfx;

    int health;
    int lives;
    int weapon;
    int score;
} collectable_t;

void collectable_init(collectable_t* collectable, collectabledef_t* def);
void collectable_destroy(collectable_t* collectable);
void collectable_step(collectable_t* collectable, float delta_time);

SHZ_FORCE_INLINE void collectable_set_position(collectable_t* collectable, shz_vec2_t pos) {
    collectable->transform.pos = pos;
}