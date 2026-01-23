#include "render_util.h"

pvr_dr_state_t g_dr_state = 0;

void render_set_sprite_header(texture_t* tex) {
    if(!tex) {
        return;
    }

    pvr_sprite_cxt_t cxt;
    pvr_sprite_cxt_txr(&cxt, PVR_LIST_TR_POLY, tex->format, tex->width, tex->height, tex->data, PVR_FILTER_NONE);
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;

    pvr_sprite_hdr_t* hdr = render_target_sprite_hdr();
    pvr_sprite_compile(hdr, &cxt);
    pvr_dr_commit(hdr);
}

void render_rect(shz_vec4_t rect, pvr_list_t list, uint32_t color) {
    pvr_sprite_cxt_t cxt;
    pvr_sprite_cxt_col(&cxt, list);
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;

    pvr_sprite_hdr_t* hdr = render_target_sprite_hdr();
    pvr_sprite_compile(hdr, &cxt);
    hdr->argb = color;
    pvr_dr_commit(hdr);

    render_textured_quad_direct(shz_vec4_init(0.0f, 0.0f, 1.0f, 1.0f), rect, 0.0f, shz_vec2_init(0.0f, 0.0f));
}

void render_textured_quad_direct(shz_vec4_t src, shz_vec4_t dst, float rotation, shz_vec2_t origin) {
    // Sprite vertices.
    shz_vec4_t p1 = { .x = 0.0f,   .y = 0.0f,  .z = 0.01f, .w = 1.0f };
    shz_vec4_t p2 = { .x = dst.z,  .y = 0.0f,  .z = 0.01f, .w = 1.0f };
    shz_vec4_t p3 = { .x = dst.z,  .y = dst.w, .z = 0.01f, .w = 1.0f };
    shz_vec4_t p4 = { .x = 0.0f,   .y = dst.w, .z = 0.01f, .w = 1.0f };

    // Transform vertices
    shz_xmtrx_init_translation(-origin.x, -origin.y, 0.0f);
    shz_xmtrx_apply_rotation_z(rotation);
    shz_xmtrx_apply_translation(dst.x, dst.y, 1.0f);
    
    p1 = shz_xmtrx_transform_vec4(p1);
    p2 = shz_xmtrx_transform_vec4(p2);
    p3 = shz_xmtrx_transform_vec4(p3);
    p4 = shz_xmtrx_transform_vec4(p4);

    float u0 = src.x;
    float v0 = src.y;
    float u1 = src.z;
    float v1 = src.w;

    // 
    pvr_sprite_txr_t* spr = render_target_sprite_txr();

    spr->flags = PVR_CMD_VERTEX_EOL;

    spr->ax = p1.x; spr->ay = p1.y; spr->az = 1.0f;
    spr->auv = PVR_PACK_16BIT_UV(u0, v0);

    spr->bx = p2.x; spr->by = p2.y; spr->bz = 1.0f;
    spr->buv = PVR_PACK_16BIT_UV(u1, v0);

    spr->cx = p3.x; spr->cy = p3.y; spr->cz = 1.0f;
    spr->cuv = PVR_PACK_16BIT_UV(u1, v1);

    spr->dx = p4.x; spr->dy = p4.y;
    pvr_dr_commit(spr);

    void* unused = render_target();
    pvr_dr_commit(unused);
}

void render_textured_quad_ext(texture_t* tex, shz_vec4_t src, shz_vec4_t dst, float rotation, shz_vec2_t origin) {
    render_set_sprite_header(tex);
    render_textured_quad_direct(src, dst, rotation, origin);
}

void render_textured_quad(texture_t* tex, shz_vec4_t src, shz_vec4_t dst, float rotation) {
    render_textured_quad_ext(tex, src, dst, rotation, shz_vec2_init(0.0f, 0.0f));
}

void render_sprite(sprite_t* sprite) {
    if(!sprite)
        return;

    shz_vec4_t dst;
    dst.xy = sprite->transform->pos;
    dst.zw = sprite->size;
    render_textured_quad_ext(sprite->tex, sprite->frame, dst, sprite->transform->rot, sprite->transform->origin);
}