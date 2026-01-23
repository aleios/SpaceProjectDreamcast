#pragma once
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/collider.h"
#include "../animator.h"

typedef struct Projectile {
    float lifetime;
    int damage;

    transform_t transform;
    sprite_t sprite;
    circlecollider_t collider;
    animator_t animator;

    shz_vec2_t velocity;
    float rotation;
    
    shz_vec2_t* target;
} projectile_t;