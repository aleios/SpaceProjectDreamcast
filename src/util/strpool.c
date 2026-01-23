#include "strpool.h"

#include <stdlib.h>
#include <string.h>

void strpool_init(strpool_t* pool, size_t capacity) {
    pool->capacity = capacity;
    pool->total = 0;
    pool->buffer = (char*)malloc(capacity);
}

void strpool_destroy(strpool_t* pool) {
    free(pool->buffer);
    pool->capacity = 0;
    pool->total = 0;
    pool->buffer = nullptr;
}

void strpool_reset(strpool_t* pool) {
    pool->total = 0;
}

const char* strpool_alloc(strpool_t* pool, const char* str) {
    if (!str || !pool->buffer)
        return NULL;

    size_t len = strlen(str) + 1;
    if (pool->total + len > pool->capacity) {
        return NULL;
    }

    char* dest = pool->buffer + pool->total;
    memcpy(dest, str, len);
    pool->total += len;
    return dest;
}