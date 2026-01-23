#pragma once
#include <kos.h>

typedef struct Texture {
    int width, height;
    pvr_ptr_t data;
    int format;
} texture_t;

bool texture_init(texture_t* tex, const char* key);
void texture_destroy(texture_t* tex);