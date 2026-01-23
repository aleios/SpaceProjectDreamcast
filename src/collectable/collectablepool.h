#pragma once

#include "collectable.h"

#define MAX_COLLECTABLES 25
typedef struct CollectablePool {
    collectable_t collectables[MAX_COLLECTABLES];
    int total;

    uint32_t pickup_sound;
} collectablepool_t;

void collectablepool_init(collectablepool_t* pool);
void collectablepool_destroy(collectablepool_t* pool);

collectable_t* collectablepool_spawn(collectablepool_t* pool, collectabletype_t type, shz_vec2_t position);
void collectablepool_despawn(collectablepool_t* pool, int index);
void collectablepool_clear(collectablepool_t* pool);

void collectablepool_step(collectablepool_t* pool, float delta_time);
void collectablepool_render(collectablepool_t* pool);
