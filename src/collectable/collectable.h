#pragma once

#include "../components/sprite.h"
#include "../components/animation.h"
#include "../components/collider.h"

typedef enum CollectableType {
    COLLECTABLE_HEALTH,
    COLLECTABLE_POWER,
    COLLECTABLE_LIFE,
    COLLECTABLE_TYPE_COUNT
} collectabletype_t;

typedef struct Collectable {
    collectabletype_t type;

    sprite_t sprite;
    transform_t transform;
    animationclip_t* clip;
    circlecollider_t collider;

    float lifetime;
} collectable_t;

void collectable_init(collectable_t* collectable, collectabletype_t type);
void collectable_destroy(collectable_t* collectable);

SHZ_FORCE_INLINE collectabletype_t collectable_get_type(collectable_t* collectable) {
    return collectable->type;
}