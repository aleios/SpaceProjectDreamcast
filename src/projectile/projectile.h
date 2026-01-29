#pragma once
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/collider.h"
#include "../animator.h"
#include "../entityid.h"
#include "../defs/projectile_data.h"

typedef struct Projectile {
    float lifetime;
    int damage;

    transform_t transform;
    sprite_t sprite;
    circlecollider_t collider;
    animator_t animator;

    float speed;
    shz_vec2_t velocity;

    bool sprite_rotates;

    entityid_t target_uid;
    projectiletrackingtype_t tracking_type;
    float targeting_delay;
    shz_vec2_t targeting_snapshot;

} projectile_t;