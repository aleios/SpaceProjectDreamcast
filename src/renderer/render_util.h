#pragma once
#include "texture.h"
#include "../components/sprite.h"
#include <sh4zam/shz_sh4zam.h>
#include <kos.h>

extern pvr_dr_state_t g_dr_state;
SHZ_FORCE_INLINE pvr_dr_state_t* render_get_state() {
    return &g_dr_state;
}

SHZ_FORCE_INLINE void* render_target() {
    return pvr_dr_target(g_dr_state);
}

SHZ_FORCE_INLINE pvr_sprite_col_t* render_target_sprite_col() {
    return (pvr_sprite_col_t*)pvr_dr_target(g_dr_state);
}

SHZ_FORCE_INLINE pvr_sprite_txr_t* render_target_sprite_txr() {
    return (pvr_sprite_txr_t*)pvr_dr_target(g_dr_state);
}

SHZ_FORCE_INLINE pvr_sprite_hdr_t* render_target_sprite_hdr() {
    return (pvr_sprite_hdr_t*)pvr_dr_target(g_dr_state);
}

SHZ_FORCE_INLINE void render_dr_begin() {
    pvr_dr_init(&g_dr_state);
}

SHZ_FORCE_INLINE void render_dr_end() {
    pvr_dr_finish();
}

void render_set_sprite_header(texture_t* tex);

void render_rect(shz_vec4_t rect, pvr_list_t list, uint32_t color);

void render_textured_quad_direct(shz_vec4_t src, shz_vec4_t dst, float rotation, shz_vec2_t origin);
void render_textured_quad_ext(texture_t* tex, shz_vec4_t src, shz_vec4_t dst, float rotation, shz_vec2_t origin);
void render_textured_quad(texture_t* tex, shz_vec4_t src, shz_vec4_t dst, float rotation);

void render_sprite(sprite_t* sprite);