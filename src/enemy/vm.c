#include "vm.h"
#include "enemy.h"

#include "../defs/gamestate.h"

static void task_finish(virtualmachine_t* vm, task_t* task) {
    if(task->is_blocking) {
        vm->is_blocked = false;
    }
}

static bool task_move_to(virtualmachine_t* vm, struct Task* task, float delta_time) {

    enemy_t* e = vm->owner;
    moveparams_t* params = &task->move.params;

    shz_vec2_t diff = shz_vec2_sub(params->target, e->transform.pos);
    float step_speed = params->speed * delta_time;

    float dist = shz_vec2_magnitude_sqr(diff);
    if(dist <= step_speed*step_speed) {
        // Snap
        e->transform.pos = params->target;
        task_finish(vm, task);

        if(task->move.destroy_on_complete) {
            enemypool_despawn(gamestate_enemy_pool(), vm->owner);
        }

        return true;
    }

    shz_vec2_t dir = shz_vec2_normalize_safe(diff);
    e->transform.pos = shz_vec2_add(e->transform.pos, shz_vec2_scale(dir, step_speed));

    return false;
}

static bool task_move_to_dir(virtualmachine_t* vm, struct Task* task, float delta_time) {

    enemy_t* e = vm->owner;
    moveparams_t* params = &task->move.params;

    float step_speed = params->speed * delta_time;

    shz_sincos_t sc = shz_sincosf(params->dir.angle);
    shz_vec2_t dir = shz_vec2_init(sc.cos, sc.sin);
    shz_vec2_t vel = shz_vec2_scale(dir, step_speed);

    params->dir.angle += params->dir.angle_step;
    task->move.accumulator += delta_time;
    if(params->dir.duration > 0.0f && task->move.accumulator >= params->dir.duration) {
        task_finish(vm, task);
        return true;
    }

    e->transform.pos = shz_vec2_add(e->transform.pos, vel);

    return false;
}

static bool task_move_to_sine(virtualmachine_t* vm, struct Task* task, float delta_time_ms) {
    enemy_t* e = vm->owner;
    moveparams_t* params = &task->move.params;

    // Convert delta_time to seconds for SHM
    // TODO: Precompute
    float dt_sec = delta_time_ms * 0.001f;

    // Forward/perpendicular vectors
    // TODO: Precompute
    shz_sincos_t sc_angle = shz_sincosf(params->sine.angle);
    shz_vec2_t forward = shz_vec2_init(sc_angle.cos, sc_angle.sin);
    shz_vec2_t perp    = shz_vec2_init(-sc_angle.sin, sc_angle.cos);

    move_sine_state_t* state = &task->move.sine_state;

    // SHM update (unit oscillator in [-1,1])
    // omega calculated in file as:
    //scale = max(0.0, min(event.get('period', 0.0), 1.0))
    // omega = 2.0 * math.pi * (scale * 15.0)

    // TODO: Precompute, all of it. It doesn't change.
    float omega2 = params->sine.omega * params->sine.omega;
    float prev_offset = state->offset;

    state->velocity += -omega2 * state->offset * dt_sec;
    state->offset   += state->velocity * dt_sec;

    float offset_delta = state->offset - prev_offset;

    // Lateral displacement in pixels
    shz_vec2_t lateral_delta = shz_vec2_scale(perp, offset_delta * params->sine.amplitude);

    // Combine forward motion (delta_time in ms) and lateral displacement
    shz_vec2_t pos_delta = shz_vec2_add(
        shz_vec2_scale(forward, params->speed * delta_time_ms), // speed is units/ms
        lateral_delta
    );

    e->transform.pos = shz_vec2_add(e->transform.pos, pos_delta);

    // Update accumulator (duration is in ms)
    task->move.accumulator += delta_time_ms;
    if (params->sine.duration > 0.0f && task->move.accumulator >= params->sine.duration) {
        task_finish(vm, task);
        return true;
    }

    return false;
}


static bool task_fire_projectile(virtualmachine_t* vm, struct Task* task, float delta_time) {
    task->fire.accumulator += delta_time;

    emitter_t* emitter = &task->fire.params.emitter;
    // emitter->lifetime = 2000.0f;
    // emitter->speed = 0.1f;
    if(task->fire.accumulator >= emitter->delay) {

        for(int i = 0; i < emitter->spawns_per_step; ++i) {
            projectilepool_spawn(gamestate_enemy_projpool(), emitter, vm->owner->transform.pos, task->fire.angle);
            task->fire.angle += emitter->step_angle;
        }

        // Wrap angle
        constexpr float inv_tau = 0.159154943f;
        constexpr float tau     = 6.283185307f;
        task->fire.angle = task->fire.angle - tau * floorf(task->fire.angle * inv_tau);

        task->fire.accumulator -= emitter->delay;
    }

    return false;
}

static bool task_delay(virtualmachine_t* vm, struct Task* task, float delta_time) {
    task->delay.accumulator += delta_time;

    if(task->delay.accumulator >= task->delay.time) {
        task_finish(vm, task);
        return true;
    }

    return false;
}

static void task_stop_movement(virtualmachine_t* vm) {
    for (int i = vm->total_tasks - 1; i >= 0; --i) {
        void* s = vm->task_queue[i].step;
        if (s == task_move_to || s == task_move_to_dir || s == task_move_to_sine) {
            if (vm->task_queue[i].is_blocking) vm->is_blocked = false;
            vm->task_queue[i] = vm->task_queue[--vm->total_tasks];
        }
    }
}

static task_t* vm_next_task(virtualmachine_t* vm) {
    return &vm->task_queue[vm->total_tasks++];
}

static void vm_fire_event(virtualmachine_t* vm, event_t ev, int event_idx) {
    if (vm->total_tasks >= MAX_TASKS) {
        printf("Task queue full!\n");
        return;
    }
        
    task_t new_task = { 0 };

    switch(ev.type) {
        case EVENT_MOVE_TO:
            new_task.move.params = ev.move;
            new_task.move.destroy_on_complete = false;

            if (ev.move.type == MOVE_DIRECTIONAL) {
                new_task.step = task_move_to_dir;
                new_task.is_blocking = (ev.move.dir.duration != 0.0f); 
            } else if(ev.move.type == MOVE_SINE) {
                new_task.step = task_move_to_sine;
                new_task.is_blocking = (ev.move.sine.duration != 0.0f);

                new_task.move.sine_state.offset = 0.0f;
                new_task.move.sine_state.velocity = ev.move.sine.omega;
            } else {
                new_task.step = task_move_to;
                // Resolve initial player position if needed
                if (ev.move.type == MOVE_PLAYER_INITIAL) {
                    new_task.move.params.target = player_get_position(gamestate_get_player());
                }
                new_task.is_blocking = true;
            }
            break;
        case EVENT_STOP_MOVING: {
            task_stop_movement(vm);
            return;
        }
        case EVENT_START_FIRING: {
            new_task.step = task_fire_projectile;
            new_task.fire.params = ev.fire;
            new_task.fire.accumulator = 0.0f;
            new_task.fire.angle = ev.fire.emitter.start_angle;
            new_task.is_blocking = false;
            break;
        }
        case EVENT_STOP_FIRING:
            for(int i = vm->total_tasks-1; i >= 0; --i) {
                if(vm->task_queue[i].step == task_fire_projectile) {
                    vm->task_queue[i] = vm->task_queue[--vm->total_tasks];
                    return;
                }
            }
            return;
        case EVENT_DELAY: {
            new_task.step = task_delay;
            new_task.delay.time = ev.delay;
            new_task.delay.accumulator = 0.0f;
            new_task.is_blocking = true;
            break;
        }
        case EVENT_DESTROY: {
            //health_add(&vm->owner->health, HEALTH_INSTANT_DEAD);
            //vm->owner->health = 0;
            enemypool_despawn(gamestate_enemy_pool(), vm->owner);
            return;
        }
        case EVENT_EXIT_SCREEN: {
            // Stop any previous movement tasks.
            task_stop_movement(vm);

            // Zoom off screen
            new_task.step = task_move_to;
            new_task.move.params.target = shz_vec2_init(vm->owner->transform.pos.x, 500.0f);
            new_task.move.params.speed = ev.exit.speed;
            new_task.move.destroy_on_complete = true;
            new_task.is_blocking = true;
            break;
        }
        case EVENT_REPEAT: {
            repeatframe_t* frame = nullptr;
            if (vm->repeat_stack_idx > 0 && vm->repeat_stack[vm->repeat_stack_idx-1].event_idx == event_idx) {
                frame = &vm->repeat_stack[vm->repeat_stack_idx-1];
            } else {
                if (vm->repeat_stack_idx >= MAX_REPEAT_DEPTH) {
                    return;
                }

                repeatframe_t new_frame = { 0 };
                new_frame.event_idx = event_idx;
                new_frame.target_event_idx = ev.repeat.target;//0; // TODO: Implement labels or relative jumps?
                new_frame.remaining_count = ev.repeat.count;
                new_frame.is_infinite = (ev.repeat.count == 0);
                vm->repeat_stack[vm->repeat_stack_idx++] = new_frame;
                frame = &vm->repeat_stack[vm->repeat_stack_idx-1];
            }

            bool should_repeat = false;
            if (frame->is_infinite) {
                should_repeat = true;
            } else if (frame->remaining_count > 0) {
                frame->remaining_count--;
                should_repeat = true;
            } else {
                vm->repeat_stack_idx--;
                should_repeat = false;
            }

            if (should_repeat) {
                // TODO: Option to stop all tasks on repeat?
                //vm->total_tasks = 0;
                //vm->is_blocked = false;
                vm->current_event = frame->target_event_idx;
            }

            return;
        }
        default:
            return;
    }

    if(new_task.is_blocking) {
        vm->is_blocked = true;
    }

    *vm_next_task(vm) = new_task;
}

void vm_init(virtualmachine_t* vm, enemy_t* enemy, event_t* event_stack, int total_events) {
    vm->owner = enemy;
    vm->is_blocked = false;
    vm->event_stack = event_stack;
    vm->total_events = total_events;
    vm->total_tasks = 0;
    vm->current_event = 0;
    vm->repeat_stack_idx = 0;
}

void vm_destroy(virtualmachine_t* vm) {
    vm->owner = nullptr;
    vm->event_stack = nullptr;
    vm->total_events = 0;
    vm->current_event = 0;
    vm->total_tasks = 0;
    vm->repeat_stack_idx = 0;
}

void vm_step(virtualmachine_t* vm, float delta_time) {
    // Process current tasks
    for(int i = 0; i < vm->total_tasks;) {
        if(vm->task_queue[i].step(vm, &vm->task_queue[i], delta_time)) {
            vm->task_queue[i] = vm->task_queue[--vm->total_tasks];
        } else {
            i++;
        }
    }

    // Process event queue
    while(!vm->is_blocked && vm->current_event < vm->total_events) {
        // Execute event
        int event_idx = vm->current_event;
        vm_fire_event(vm, vm->event_stack[event_idx], event_idx);

        if (vm->current_event == event_idx) {
            ++vm->current_event;
        }
    }
}

/*
Example stack:
    - START_FIRING()
    - MOVE_TO(POINT, 32, 32)
    - STOP_FIRING()

*/