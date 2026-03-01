#include "render_util.h"

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

void render_circle(shz_vec2_t pos, float radius, pvr_list_t list, uint32_t color) {
    constexpr int steps = 14;
    constexpr float step_delta = 2.0f * SHZ_F_PI / steps;

    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_col(&cxt, list);
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;

    const auto hdr = render_target_poly_hdr();
    pvr_poly_compile(hdr, &cxt);
    pvr_dr_commit(hdr);

    const auto vert = render_target_vertex();
    vert->argb = color;
    vert->oargb = 0;

    float angle = 0;
    for (int i = 0; i < steps; ++i) {
        const float x1 = pos.x + cosf(angle) * radius;
        const float y1 = pos.y + sinf(angle) * radius;
        const float x2 = pos.x + cosf(angle + step_delta) * radius;
        const float y2 = pos.y + sinf(angle + step_delta) * radius;

        vert->flags = PVR_CMD_VERTEX;
        vert->x = x1;
        vert->y = y1;
        vert->z = 1.0f;
        pvr_dr_commit(vert);

        vert->flags = PVR_CMD_VERTEX;
        vert->x = x2;
        vert->y = y2;
        vert->z = 1.0f;
        pvr_dr_commit(vert);

        vert->flags = PVR_CMD_VERTEX_EOL;
        vert->x = pos.x;
        vert->y = pos.y;
        vert->z = 1.0f;
        pvr_dr_commit(vert);

        angle += step_delta;
    }
}

void render_textured_quad_direct(shz_vec4_t src, shz_vec4_t dst, float rotation, shz_vec2_t origin) {
    // Sprite vertices.
    shz_vec2_t p1 = shz_vec2_init(0.0f, 0.0f);
    shz_vec2_t p2 = shz_vec2_init(dst.z, 0.0f);
    shz_vec2_t p3 = shz_vec2_init(dst.z, dst.w);
    shz_vec2_t p4 = shz_vec2_init(0.0f, dst.w);

    // Transform vertices
    shz_xmtrx_init_translation(dst.x, dst.y, 0.0f);
    shz_xmtrx_apply_rotation_z(rotation);
    shz_xmtrx_translate(-origin.x, -origin.y, 0.0f);

    p1 = shz_xmtrx_transform_point2(p1);
    p2 = shz_xmtrx_transform_point2(p2);
    p3 = shz_xmtrx_transform_point2(p3);
    p4 = shz_xmtrx_transform_point2(p4);

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

    // Note: sprite is 64-bytes, commit does 32, so submit the last 32.
    // It's already laid out in SQ2.
    void* spr_continued = render_target();
    pvr_dr_commit(spr_continued);
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