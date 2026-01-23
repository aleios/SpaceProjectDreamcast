#pragma once
#include "transform.h"
#include "../renderer/texture.h"

typedef struct Sprite {
    texture_t* tex;
    transform_t* transform;
    shz_vec4_t frame;
    shz_vec2_t size;
    bool visible;
} sprite_t;

void sprite_init(sprite_t* sprite, transform_t* transform, texture_t* tex);
void sprite_destroy(sprite_t* sprite);

void sprite_set_origin(sprite_t* sprite, shz_vec2_t origin);

SHZ_FORCE_INLINE void sprite_set_visibility(sprite_t* sprite, bool visible) {
    sprite->visible = visible;
}