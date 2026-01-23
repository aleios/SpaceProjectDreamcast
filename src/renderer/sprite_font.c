#include "sprite_font.h"
#include "render_util.h"
#include "../cache/caches.h"
#include "../util/readutils.h"

bool spritefont_init(spritefont_t* font, const char* key) {

    char path_buf[256];
    if (!path_build_rd(path_buf, sizeof(path_buf), "fonts", key, "dat")) {
        return false;
    }

    // aaa
    file_t f = fs_open(path_buf, O_RDONLY);
    if (f < 0) {
        return false;
    }

    // Read tex name
    if (!readutil_readstr(f, path_buf, sizeof(path_buf))) {
        fs_close(f);
        return false;
    }
    // Read cell width and height
    uint16_t cell_w, cell_h;
    fs_read(f, &cell_w, sizeof(cell_w));
    fs_read(f, &cell_h, sizeof(cell_h));

    // Done with the file.
    fs_close(f);

    font->tex = texcache_get(path_buf);

    if(!font->tex)
        return false;

    if(font->tex->width == 0 || font->tex->height == 0) {
        return false;
    }

    font->cell_width = cell_w;
    font->cell_height = cell_h;

    int cols = font->tex->width / font->cell_width;
    for(int i = 0; i < FONT_END_SIZE; ++i) {

        int xidx = i % cols;
        int yidx = i / cols;

        font->glyphs[i] = shz_vec4_init(
            (float)(xidx * font->cell_width) / font->tex->width,
            (float)(yidx * font->cell_height) / font->tex->height,
            (float)((xidx+1) * font->cell_width) / font->tex->width,
            (float)((yidx+1) * font->cell_height) / font->tex->height
        );
    }

    pvr_sprite_cxt_t cxt;
    pvr_sprite_cxt_txr(&cxt, PVR_LIST_TR_POLY, font->tex->format,
        font->tex->width, font->tex->height, font->tex->data,
        PVR_FILTER_NONE);
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    pvr_sprite_compile(&font->hdr, &cxt);

    return true;
}

void spritefont_destroy(spritefont_t* font) {
    font->cell_height = 0;
    font->cell_height = 0;
    texcache_release(font->tex);
}

SHZ_FORCE_INLINE shz_vec4_t spritefont_get_glyph(spritefont_t* font, char c) {
    return font->glyphs[(int)c - FONT_CHR_OFFSET];
}

void spritefont_render(spritefont_t* font, const char* text, shz_vec2_t pos, uint32_t color) {
    pvr_sprite_hdr_t* hdr = render_target_sprite_hdr();
    *hdr = font->hdr;
    hdr->argb = color;
    pvr_dr_commit(hdr);

    size_t len = strlen(text);
    for(int i = 0; i < len; ++i) {

        switch(text[i]) {
        case ' ':
            pos.x += font->cell_width;
            continue;
        case '\n':
            pos.y += font->cell_height;
            pos.x = 0.0f;
            continue;
        default:
            break;    
        }

        shz_vec4_t g = spritefont_get_glyph(font, text[i]);
        render_textured_quad_direct(g, shz_vec4_init(pos.x, pos.y, font->cell_width, font->cell_height), 0.0, shz_vec2_init(0.0f,0.0f));
        pos.x += font->cell_width;
    }
}

shz_vec2_t spritefont_str_size(spritefont_t* font, const char* text) {
    size_t len = strlen(text);

    if(len <= 0) {
        return shz_vec2_init(0.0f, 0.0f);
    }

    float xdim = 0.0f;
    shz_vec2_t pen = shz_vec2_init(0.0f, font->cell_height);
    for(int i = 0; i < len; ++i) {
        if(text[i] == '\n') {
            pen.y += font->cell_height;
            continue;
        }
        pen.x += font->cell_width;

        if(pen.x > xdim) {
            xdim = pen.x;
        }
    }

    return shz_vec2_init(xdim, pen.y);
}