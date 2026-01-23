#include "animation.h"
#include "../util/readutils.h"
#include <kos.h>

bool animation_init(animation_t* anim, const char* key) {
    if (!anim || !key) {
        return false;
    }

    char path[256] = {0};
    path_build_rd(path, sizeof(path), "animations", key, "anim");

    file_t anim_file = fs_open(path, O_RDONLY);
    if (anim_file <= -1) {
        printf("[Animation] Error: Could not open file: %s\n", path);
        return false;
    }

    char magic_num[4];
    uint16_t total_clips = 0;

    // Read and verify header
    if (fs_read(anim_file, magic_num, 4) != 4 || 
        memcmp(magic_num, "ASAM", 4) != 0) {
        printf("[Animation] Error: Invalid magic number in %s\n", path);
        goto error_close;
    }

    if (fs_read(anim_file, &total_clips, sizeof(uint16_t)) != sizeof(uint16_t)) {
        goto error_close;
    }

    anim->total_clips = total_clips;
    anim->clips = malloc(sizeof(animationclip_t) * total_clips);
    if (!anim->clips) {
        goto error_close;
    }

    char name_buffer[256];
    for (int i = 0; i < total_clips; ++i) {
        animationclip_t* clip = &anim->clips[i];

        clip->name = nullptr;
        clip->frames = nullptr;

        if (!readutil_readstr(anim_file, name_buffer, sizeof(name_buffer))) {
            anim->total_clips = i;
            goto error_destroy;
        }
        clip->name = strdup(name_buffer);

        // Read in properties.
        fs_read(anim_file, &clip->frame_time, sizeof(float));
        fs_read(anim_file, &clip->loop_mode, sizeof(uint8_t));
        fs_read(anim_file, &clip->origin, sizeof(shz_vec2_t));
        fs_read(anim_file, &clip->total_frames, sizeof(uint16_t));

        // Sanity check
        if (clip->total_frames == 0 || clip->total_frames > 1024) {
             anim->total_clips = i + 1;
             goto error_destroy;
        }

        clip->frames = malloc(sizeof(animationframe_t) * clip->total_frames);
        if (!clip->frames) {
            anim->total_clips = i + 1;
            goto error_destroy;
        }

        // Read frames
        const size_t frames_size = sizeof(animationframe_t) * clip->total_frames;
        if (fs_read(anim_file, clip->frames, frames_size) != frames_size) {
            anim->total_clips = i + 1;
            goto error_destroy;
        }
    }

    fs_close(anim_file);
    return true;

error_destroy:
    animation_destroy(anim);
error_close:
    fs_close(anim_file);
    return false;
}

void animation_destroy(animation_t* anim) {

    for(int i = 0; i < anim->total_clips; ++i) {
        free(anim->clips[i].name);
        free(anim->clips[i].frames);
    }
    free(anim->clips);
    anim->clips = nullptr;
    anim->total_clips = 0;
}

animationclip_t* animation_get_clip(animation_t* anim, const char* name) {
    for(int i = 0; i < anim->total_clips; ++i) {
        if(strcmp(name, anim->clips[i].name) == 0) {
            return &anim->clips[i];
        }
    }
    return nullptr;
}