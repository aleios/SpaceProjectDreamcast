#pragma once
#include <kos.h>
#include <sh4zam/shz_sh4zam.h>
#include "projectile_def.h"

typedef struct Emitter {
    projectiledef_t* def;
    int spawns_per_step;
    float delay;
    float start_angle;
    float step_angle;
    float speed;
    float lifetime;
    shz_vec2_t offset;

    struct {
        float fire_timer;
    } runtime;
} emitter_t;

bool emitter_read(emitter_t* emitter, file_t file);