#pragma once
#include <stddef.h>

typedef struct StrPool {
    char* buffer;
    size_t capacity;
    size_t total;
} strpool_t;

void strpool_init(strpool_t* pool, size_t capacity);
void strpool_destroy(strpool_t* pool);
void strpool_reset(strpool_t* pool);

const char* strpool_alloc(strpool_t* pool, const char* str);