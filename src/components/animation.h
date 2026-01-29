#pragma once
#include <stdint.h>
#include <sh4zam/shz_sh4zam.h>
#include "../renderer/texture.h"

enum AnimationLoopMode {
    ANIMATION_LOOP_FORWARD,
    ANIMATION_LOOP_BACKWARD,
    ANIMATION_LOOP_PING_PONG
};

typedef struct AnimationFrame {
    shz_vec4_t uv;
    shz_vec2_t size;
} animationframe_t;

typedef struct AnimationClip {
    char* name;
    float frame_time;
    animationframe_t* frames;
    uint16_t total_frames;
    uint8_t loop_mode;
    shz_vec2_t origin;
} animationclip_t;

typedef struct Animation {
    texture_t* tex;
    animationclip_t* clips;
    uint16_t total_clips;
} animation_t;

bool animation_init(animation_t* anim, const char* key);
void animation_destroy(animation_t* anim);

animationclip_t* animation_get_clip(animation_t* anim, const char* name);
SHZ_FORCE_INLINE texture_t* animation_get_texture(animation_t* anim) {
    return anim->tex;
}