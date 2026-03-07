#pragma once

typedef enum Direction {
    DIRECTION_NONE = -1,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_UP,
    DIRECTION_DOWN,
} direction_t;

void direction_register_lua();