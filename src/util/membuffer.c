#include "membuffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <arch/arch.h>
#include <sh4zam/shz_sh4zam.h>

static void membuffer_ensure(membuffer_t* buf, int length) {
    if (buf->len + length < buf->capacity) {
        return;
    }
    buf->capacity *= 2;
    const auto new_buf = realloc(buf->data, sizeof(char) * buf->capacity);
    if (!new_buf) {
        arch_abort();
    }
    buf->data = new_buf;
}

void membuffer_init(membuffer_t* buf, int initial_capacity) {
    assert(initial_capacity > 1);

    buf->data = malloc(sizeof(char) * initial_capacity);
    buf->len = 0;
    buf->capacity = initial_capacity;
}

void membuffer_destroy(membuffer_t* buf) {
    if (buf->data) {
        free(buf->data);
    }
    buf->data = nullptr;
    buf->len = 0;
    buf->capacity = 0;
}

void membuffer_write(membuffer_t* buf, const char* data, int len) {
    membuffer_ensure(buf, len);
    shz_memcpy(&buf->data[buf->len], data, len);
    buf->len += len;
}

void membuffer_shrink(membuffer_t* buf) {
    if (!buf->data || buf->len == buf->capacity) {
        return;
    }

    const auto new_buf = realloc(buf->data, sizeof(char) * buf->len);
    if (!new_buf) {
        arch_abort();
    }
    buf->data = new_buf;
    buf->capacity = buf->len;
}