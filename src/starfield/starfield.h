#pragma once
#include <kos.h>
#include <sh4zam/shz_sh4zam.h>

#define STARFIELD_LAYERS 3
#define STARFIELD_DEFAULT_SPEED 0.08f
typedef struct StarfieldLayer {
    shz_vec2_t* stars;
    int total_stars;
} starfield_layer_t;

typedef struct Starfield {
    starfield_layer_t layers[STARFIELD_LAYERS];
    float base_speed;

    pvr_sprite_hdr_t hdr;
} starfield_t;

void starfield_init(starfield_t* field, int min_stars, int max_stars);
void starfield_destroy(starfield_t* field);

void starfield_step(starfield_t* field, float delta_time);
void starfield_render(starfield_t* field);

SHZ_FORCE_INLINE void starfield_set_speed(starfield_t* field, float speed) {
    field->base_speed = speed;
}