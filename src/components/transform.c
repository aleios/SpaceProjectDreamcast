#include "transform.h"

void transform_init(transform_t* transform) {
    transform->pos = (shz_vec2_t){ .x = 0.0f, .y = 0.0f };
    transform->rot = 0.0f;
    transform->origin = (shz_vec2_t){ .x = 0.0f, .y = 0.0f };
}