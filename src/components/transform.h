#pragma once
#include <sh4zam/shz_sh4zam.h>

typedef struct Transform {
    shz_vec2_t pos;
    float rot;
    shz_vec2_t origin;
} transform_t;

void transform_init(transform_t* transform);