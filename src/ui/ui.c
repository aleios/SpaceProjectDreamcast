#include "ui.h"
#include "../renderer/render_util.h"
#include "../defs/gamestate.h"
#include "../cache/caches.h"

void ui_init(ui_t* ui) {

    ui->tex = texcache_get("ui");
    ui->anim = animcache_get("ui");

    animationclip_t* health_clip = animation_get_clip(ui->anim, "healthbar");
    animationclip_t* healthpip_clip = animation_get_clip(ui->anim, "healthpip");
    animationclip_t* life_clip = animation_get_clip(ui->anim, "life");

    ui->healthbar_frame = health_clip->frames[0];
    ui->healthpip_frame = healthpip_clip->frames[0];
    ui->life_frame = life_clip->frames[0];
    ui->font = fontcache_get("main_font");
}

void ui_destroy(ui_t* ui) {
    fontcache_release(ui->font);
    // animcache_release(ui->ui_anim);
    // texcache_release(ui->ui_tex);
}

void ui_render(ui_t* ui) {

    // TODO: Cache this crap.
    shz_vec2_t healthbar_size = ui->healthbar_frame.size;
    shz_vec4_t healthbar_pos = shz_vec4_init(0.0f, 0.0f, healthbar_size.x * 2.0f, healthbar_size.y * 2.0f);
    render_textured_quad(ui->tex, ui->healthbar_frame.uv, healthbar_pos, 0.0f);

    for(int i = 0; i < g_gamestate.health; ++i) {
        shz_vec2_t healthpip_size = ui->healthpip_frame.size;
        render_textured_quad(ui->tex, ui->healthpip_frame.uv, shz_vec4_init(
            4.0f + (((healthpip_size.x * 2.0f) + 2.0f) * i),
            14.0f,
            healthpip_size.x * 2.0f,
            healthpip_size.y * 2.0f
        ), 0.0f);
    }

    for(int i = 0; i < g_gamestate.lives; ++i) {
        shz_vec2_t life_size = ui->life_frame.size;
        render_textured_quad(ui->tex, ui->life_frame.uv, shz_vec4_init(
                0.0f + ((life_size.x * i) + 2.0f),
                healthbar_size.y * 2.0f + 5.0f,
                life_size.x, 
                life_size.y
            ), 0.0f
        );
    }

    float y = (healthbar_size.y * 2.0f + 5.0f) + ui->life_frame.size.y;
    char score_buf[50];
    snprintf(score_buf, sizeof(score_buf), "Score: %d", g_gamestate.score);
    spritefont_render(ui->font, score_buf, shz_vec2_init(0.0f, y), 0xFFFFFFFF);
}