#include "sprite_renderer.h"
#include "../renderer/render_util.h"

#define MAX_SPRITES 128
typedef struct SpriteRenderer {
    sprite_t* sprites[MAX_SPRITES];
    int active_sprites;
} spriterenderer_t;
spriterenderer_t g_spriteRenderer;

void sprite_renderer_add(sprite_t* sprite) {
    if(g_spriteRenderer.active_sprites >= MAX_SPRITES) {
        return;
    }

    g_spriteRenderer.sprites[g_spriteRenderer.active_sprites++] = sprite;
}

void sprite_renderer_remove(sprite_t* sprite) {
    for (int i = 0; i < g_spriteRenderer.active_sprites; ++i) {
        if (g_spriteRenderer.sprites[i] == sprite) {
            // Swap with the last active sprite to keep the array packed
            g_spriteRenderer.sprites[i] = g_spriteRenderer.sprites[g_spriteRenderer.active_sprites - 1];
            g_spriteRenderer.sprites[g_spriteRenderer.active_sprites - 1] = nullptr;
            g_spriteRenderer.active_sprites--;
            return;
        }
    }
}

void sprite_renderer_render() {
    for(int i = 0; i < g_spriteRenderer.active_sprites; ++i) {
        sprite_t* sprite = g_spriteRenderer.sprites[i];

        render_sprite(sprite);
    }
}