#pragma once
#include "../defs/projectile_def.h"
#include "../defs/emitter.h"

#define MAX_EVENTS 25

typedef enum MoveType {
    MOVE_POINT,
    MOVE_PLAYER_INITIAL,
    MOVE_PLAYER_TARGET,
    MOVE_DIRECTIONAL,
    MOVE_SINE,
} movetype_t;

typedef struct MoveParams {
    movetype_t type;
    float speed;
    union {
        shz_vec2_t target;
        struct {
            float angle;
            float angle_step;
            float duration;
        } dir;
        struct {
            float omega;
            float amplitude;
            float angle;
            float duration;
        } sine;
    };
} moveparams_t;

typedef struct FireParams {
    emitter_t emitter;
} fireparams_t;

typedef struct ExitScreenParams {
    float speed;
} exitscreenparams_t;

typedef struct RepeatParams {
    int count;
    int target;
} repeatparams_t;

typedef enum EventType {
    EVENT_MOVE_TO = 0,
    EVENT_STOP_MOVING,
    EVENT_START_FIRING,
    EVENT_STOP_FIRING,
    EVENT_DELAY,
    EVENT_DESTROY,
    EVENT_EXIT_SCREEN,
    EVENT_REPEAT
} eventtype_t;

typedef struct Event {
    eventtype_t type;
    union {
        moveparams_t move;
        fireparams_t fire;
        float delay;
        exitscreenparams_t exit;
        repeatparams_t repeat;
    };
} event_t;