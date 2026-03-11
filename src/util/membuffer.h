#pragma once

#include <sh4zam/shz_cdefs.h>

typedef struct MemBuffer {
    char* data;
    int len;
    int capacity;
} membuffer_t;

void membuffer_init(membuffer_t* buf, int initial_capacity);
void membuffer_destroy(membuffer_t* buf);

void membuffer_write(membuffer_t* buf, const char* data, int len);

void membuffer_shrink(membuffer_t* buf);

SHZ_FORCE_INLINE int membuffer_len(membuffer_t* buf) {
    return buf->len;
}

SHZ_FORCE_INLINE int membuffer_capacity(membuffer_t* buf) {
    return buf->capacity;
}

SHZ_FORCE_INLINE char* membuffer_data(membuffer_t* buf) {
    return buf->data;
}