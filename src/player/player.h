#pragma once
#include "../components/sprite.h"
#include "../animator.h"
#include "../components/collider.h"

typedef struct Player {
    sprite_t sprite;
    transform_t transform;
    circlecollider_t collider;
    
    float speed;

    bool firing;

    animator_t animator;
    animation_t* anim;
    animationclip_t* clip_idle;
    animationclip_t* clip_left;
    animationclip_t* clip_right;

    uint32_t boom;
} player_t;

void player_init(player_t* player);
void player_destroy(player_t* player);

void player_step(player_t* player, float delta_time);

void player_set_position(player_t* player, shz_vec2_t pos);
void player_move(player_t* player, shz_vec2_t offset);

SHZ_FORCE_INLINE shz_vec2_t player_get_position(player_t* player) {
    return player->transform.pos;
}