#include "collider.h"
#include <stdio.h>

bool collider_test_box(boxcollider_t* a, boxcollider_t* b) {
    return !(a->x > b->x + b->z ||
             b->x > a->x + a->z ||
             a->y > b->y + b->w ||
             b->y > a->y + a->w);
}

bool collider_test_circle(circlecollider_t* a, circlecollider_t* b) {

    float dist_x = a->center.x - b->center.x;
    float dist_y = a->center.y - b->center.y;

    shz_vec4_t d = shz_vec4_init(dist_x, dist_y, 0.0f, 0.0f);
    float dist_sq = shz_vec4_dot(d, d);
    float rad_sum = a->radius + b->radius;
    return dist_sq <= (rad_sum * rad_sum);

    // float dist = shz_vec2_distance_sqr(a->center, b->center);
    // float rad_sum = a->radius + b->radius;
    // return dist <= (rad_sum * rad_sum);
}

bool collider_test_circle_box(circlecollider_t* a, boxcollider_t* b) {
    float close_x = shz_clampf(a->center.x, b->x, b->x + b->z);
    float close_y = shz_clampf(a->center.y, b->y, b->y + b->w);

    float dist_x = a->center.x - close_x;
    float dist_y = a->center.y - close_y;

    shz_vec4_t d = shz_vec4_init(dist_x, dist_y, 0.0f, 0.0f);
    float dist_sq = shz_vec4_dot(d, d);
    return dist_sq <= (a->radius * a->radius);


    //shz_vec2_t test_pos = a->center;
    // if(test_pos.x < b->x) {
    //     test_pos.x = b->x;
    // } else if(test_pos.x > b->x + b->z) {
    //     test_pos.x = b->x + b->z;
    // }
    
    // if(test_pos.y < b->y) {
    //     test_pos.y = b->y;
    // } else if(test_pos.y > b->y + b->w) {
    //     test_pos.y = b->y + b->w;
    // }
    // return (shz_vec2_distance_sqr(a->center, test_pos) <= a->radius * a->radius);
}

bool collider_test_point_box(shz_vec2_t point, shz_vec4_t box) {
    return point.x >= box.x && 
           point.x <= box.x + box.z && 
           point.y >= box.y && 
           point.y <= box.y + box.w;
}