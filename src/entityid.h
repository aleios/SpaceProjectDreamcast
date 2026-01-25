#pragma once
#include <stdint.h>

enum EntityID {
    ENTITY_NULL = 0,
    ENTITY_PLAYER = 1,
    ENTITY_DYNAMIC_START
};
typedef uint32_t entityid_t;