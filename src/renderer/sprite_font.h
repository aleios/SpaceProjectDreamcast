#pragma once
#include "texture.h"
#include <sh4zam/shz_sh4zam.h>
#include <kos.h>

#define FONT_END_SIZE   0x5E
#define FONT_CHR_OFFSET 0x20

typedef struct SpriteFont {
    texture_t* tex;
    int cell_width, cell_height;
    shz_vec4_t glyphs[FONT_END_SIZE];
    pvr_sprite_hdr_t hdr;
} spritefont_t;

bool spritefont_init(spritefont_t* font, const char* key);
void spritefont_destroy(spritefont_t* font);

void spritefont_render(spritefont_t* font, const char* text, shz_vec2_t pos, uint32_t color);

shz_vec2_t spritefont_str_size(spritefont_t* font, const char* text);

SHZ_FORCE_INLINE int spritefont_cell_width(spritefont_t* font) {
    return font->cell_width;
}

SHZ_FORCE_INLINE int spritefont_cell_height(spritefont_t* font) {
    return font->cell_height;
}