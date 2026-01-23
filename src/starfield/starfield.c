#include "starfield.h"
#include "../util/randgen.h"
#include "../globals.h"
#include "../renderer/render_util.h"

// Source: https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
static uint32_t wang_hash(uint32_t x) {
    x = (x ^ 61u) ^ (x >> 16);
    x = x * 9u;
    x = x ^ (x >> 4);
    x = x * 0x27d4eb2du;
    x = x ^ (x >> 15);
    return x;
}

#define STARFIELD_LAYER_INCREMENT 0.035f
#define STARFIELD_JITTER_AMPLITUDE 0.015f

void starfield_init(starfield_t* field, int min_stars, int max_stars) {
    field->base_speed = STARFIELD_DEFAULT_SPEED;

    for(int i = 0; i < STARFIELD_LAYERS; ++i) {
        starfield_layer_t* layer = &field->layers[i];

        layer->total_stars = rand_between(min_stars, max_stars);
        layer->stars = malloc(sizeof(shz_vec2_t) * layer->total_stars);

        for(int j = 0; j < layer->total_stars; ++j) {
            layer->stars[j] = shz_vec2_init(
                rand_between(0, SCREEN_WIDTH-1),
                rand_between(-80, SCREEN_HEIGHT-1)
            );
        }
    }

    pvr_sprite_cxt_t starfield_cxt;
    pvr_sprite_cxt_col(&starfield_cxt, PVR_LIST_OP_POLY);
    starfield_cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    starfield_cxt.gen.culling = PVR_CULLING_NONE;
    pvr_sprite_compile(&field->hdr, &starfield_cxt);
}

void starfield_destroy(starfield_t* field) {

    for (int i = 0; i < STARFIELD_LAYERS; ++i) {
        free(field->layers[i].stars);
        field->layers[i].stars = nullptr;
        field->layers[i].total_stars = 0;
    }
}

void starfield_step(starfield_t* field, float delta_time) {

    for(int i = 0; i < STARFIELD_LAYERS; ++i) {
        starfield_layer_t* layer = &field->layers[i];

        const float layer_base_speed = field->base_speed + (i * STARFIELD_LAYER_INCREMENT);

        for(int j = 0; j < layer->total_stars; ++j) {
            shz_vec2_t* star = &layer->stars[j];

            uint32_t hash = wang_hash(((uint32_t)i << 20) ^ (uint32_t)j);
            float u = (float)(hash & 0xFFFFFF) / (float)0xFFFFFF; // Between 0 and 1
            float jitter = (u * 2.0f - 1.0f) * STARFIELD_JITTER_AMPLITUDE;

            float spd = layer_base_speed + jitter;
            star->y += spd * delta_time;

            if (star->y >= SCREEN_HEIGHT) {
                star->y = -rand_between(10, 300);
            }
        }
    }
}

void starfield_render(starfield_t* field) {

    for(int layer_id = 0; layer_id < STARFIELD_LAYERS; ++layer_id) {
        starfield_layer_t* layer = &field->layers[layer_id];

        // Submit header
        pvr_sprite_hdr_t* hdr = render_target_sprite_hdr();
        *hdr = field->hdr;

        switch(layer_id) {
        case 0:
            hdr->argb = 0xFF444444;
            break;
        case 1:
            hdr->argb = 0xFF666666;
            break;
        default:
            hdr->argb = 0xFF888888;
            break;
        }
        pvr_dr_commit(hdr);

        for(int j = 0; j < layer->total_stars; ++j) {
            // Submit sprites
            float xpos = layer->stars[j].x;
            float ypos = layer->stars[j].y;

            if(ypos < -1.0f || ypos > SCREEN_HEIGHT+1.0f) {
                continue;
            }

            float sz = 0.75f + (layer_id * 0.4f);
            pvr_sprite_col_t* vert = render_target_sprite_col();
            vert->flags = PVR_CMD_VERTEX_EOL;
            vert->ax = xpos; vert->ay = ypos; vert->az = 2.0f + layer_id;
            vert->bx = xpos + sz; vert->by = ypos; vert->bz = 2.0f + layer_id;
            vert->cx = xpos + sz; vert->cy = ypos + sz; vert->cz = 2.0f + layer_id;
            vert->dx = xpos; vert->dy = ypos + sz;
            pvr_dr_commit(vert);

            void* tmp = render_target();
            pvr_dr_commit(tmp);
        }
    }
}