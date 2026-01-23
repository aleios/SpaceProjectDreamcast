#pragma once
#include "../defs/projectile_def.h"
#include "enemy_event.h"

#define MAX_TASKS 25
#define MAX_REPEAT_DEPTH 10

typedef struct MoveSineState {
    float offset;
    float velocity;

    shz_vec2_t fwd;
    shz_vec2_t perp;
} move_sine_state_t;

typedef struct Enemy enemy_t;
typedef struct VirtualMachine virtualmachine_t;
typedef struct Task {
    bool (*step)(virtualmachine_t* vm, struct Task* task, float delta_time);
    union {
        struct {
            moveparams_t params;
            float accumulator;
            move_sine_state_t sine_state;
            bool destroy_on_complete;
        } move;
        struct { 
            fireparams_t params; 
            float accumulator;
            float angle;
        } fire;
        struct { 
            float time; 
            float accumulator; 
        } delay;
    };
    bool is_blocking;
} task_t;

typedef struct RepeatFrame {
    int event_idx;
    int target_event_idx;
    int remaining_count;
    bool is_infinite;
} repeatframe_t;

typedef struct VirtualMachine {
    enemy_t* owner;

    //event_t event_stack[MAX_EVENTS];
    event_t* event_stack;
    int total_events;
    int current_event;

    task_t task_queue[MAX_TASKS];
    int total_tasks;

    bool is_blocked;

    repeatframe_t repeat_stack[MAX_REPEAT_DEPTH];
    int repeat_stack_idx;
} virtualmachine_t;

void vm_init(virtualmachine_t* vm, enemy_t* enemy, event_t* event_stack, int total_events);
void vm_destroy(virtualmachine_t* vm);
void vm_step(virtualmachine_t* vm, float delta_time);