#pragma once
#include <sh4zam/shz_sh4zam.h>

typedef shz_vec4_t boxcollider_t;

typedef struct CircleCollider {
    shz_vec2_t center;
    float radius;
} circlecollider_t;

bool collider_test_box(boxcollider_t* a, boxcollider_t* b);
bool collider_test_circle(circlecollider_t* a, circlecollider_t* b);
bool collider_test_circle_box(circlecollider_t* a, boxcollider_t* b);
bool collider_test_point_box(shz_vec2_t point, shz_vec4_t box);