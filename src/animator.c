#include "animator.h"

void animator_init(animator_t* anim, sprite_t* sprite, animationclip_t* initial_clip) {
    animator_reset(anim);
    anim->sprite = sprite;
    anim->clip = nullptr;
    animator_set_clip(anim, initial_clip);
}

void animator_destroy(animator_t* anim) {
    animator_reset(anim);
    anim->sprite = nullptr;
    anim->clip = nullptr;
}

void animator_reset(animator_t* anim) {
    anim->accumulator = 0.0f;
    anim->current_frame = 0;
}

void animator_set_clip(animator_t* anim, animationclip_t* clip) {
    if(anim->clip == clip) {
        return;
    }

    anim->clip = clip;
    animator_reset(anim);

    if (clip) { // Backward mode
        if(clip->loop_mode == ANIMATION_LOOP_BACKWARD) {
            anim->current_frame = clip->total_frames - 1;
            anim->direction = -1;
        } else {
            anim->direction = 1;
        }

        if(anim->sprite) {
            if(clip->total_frames > 0) {
                anim->sprite->frame = anim->clip->frames[0].uv;
                anim->sprite->size  = anim->clip->frames[0].size;
            }
            sprite_set_origin(anim->sprite, clip->origin);
        }
    }
}

void animator_step(animator_t* anim, float delta_time) {
    if(!anim || !anim->clip || !anim->sprite) {
        return;
    }

    animationclip_t* clip = anim->clip;
    if(clip->total_frames == 1 || clip->frame_time <= 0.0f) {
        anim->sprite->frame = clip->frames[0].uv;
        anim->sprite->size = clip->frames[0].size;
        return;
    }

    anim->accumulator += delta_time;
    if(anim->accumulator >= clip->frame_time) {

        switch (clip->loop_mode) {
        case ANIMATION_LOOP_BACKWARD:
            if (anim->current_frame == 0) {
                anim->current_frame = clip->total_frames - 1;
            } else {
                anim->current_frame--;
            }
            break;
        case ANIMATION_LOOP_PING_PONG:
            anim->current_frame += anim->direction;

            if (anim->current_frame >= (int)clip->total_frames) {
                anim->direction = -1;
                anim->current_frame = clip->total_frames - 2;
                if (anim->current_frame < 0) anim->current_frame = 0;
            } else if (anim->current_frame < 0) {
                anim->direction = 1;
                anim->current_frame = 1;
                if (anim->current_frame >= (int)clip->total_frames) anim->current_frame = 0;
            }
            break;
        default:
            anim->current_frame++;
            if (anim->current_frame >= clip->total_frames) {
                anim->current_frame = 0;
            }
            break;
        }

        anim->sprite->frame = clip->frames[anim->current_frame].uv;
        anim->sprite->size  = clip->frames[anim->current_frame].size;

        anim->accumulator -= clip->frame_time;
    }
}

void animator_refresh(animator_t* anim) {
    if(!anim || !anim->clip) {
        return;
    }
    // Resync animator states.
    sprite_set_origin(anim->sprite, anim->clip->origin);
}