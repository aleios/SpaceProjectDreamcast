#include "sprite.h"

void sprite_init(sprite_t* sprite, transform_t* transform, texture_t* tex) {
    sprite->transform = transform;
    sprite->tex = tex;
    sprite->visible = true;
}

void sprite_destroy(sprite_t* sprite) {
    sprite->transform = nullptr;
    sprite->tex = nullptr;
    sprite->visible = false;
}

void sprite_set_origin(sprite_t* sprite, shz_vec2_t origin) {
    if(sprite && sprite->transform) {
        sprite->transform->origin = origin;
    }
}