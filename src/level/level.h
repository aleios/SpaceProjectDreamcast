#pragma once
#include "../enemy/enemy_def.h"
#include "../util/strpool.h"

typedef enum LevelEventType {
    LEVEL_EVENT_SPAWN,             // <| Spawn an enemy at position
    LEVEL_EVENT_MUSIC,             // <| Change to music
    LEVEL_EVENT_WAITCLEAR,         // <| Block until all current enemies despawned
    LEVEL_EVENT_DELAY,             // <| Delay further events for x ms
    LEVEL_EVENT_STARFIELD_SPEED,   // <| Set background starfield speed
    LEVEL_EVENT_CLEAR,             // <| Clear active projectiles and/or enemies.
} leveleventtype_t;

typedef struct EnemyTimePair {
    enemydef_t* def;
    uint64_t time;
} enemy_timepair_t;

typedef struct SpawnerParams {
    enemydef_t* def;
} spawnerparams_t;

typedef struct MusicParams {
    const char* key;
    float fade_in;
    float fade_out;
} musicparams_t;

typedef struct DelayParams {
    float duration;
} delayparams_t;

typedef struct StarfieldParams {
    float speed;
    float duration;
    bool block;
} starfieldparams_t;

typedef struct WaitClearParams {
    float timeout;
} waitclearparams_t;

typedef struct ClearParams {
    bool player_projectiles;
    bool enemy_projectiles;
    bool enemies;
    bool collectables;
} clearparams_t;

typedef struct LevelEvent {
    leveleventtype_t type;
    union {
        spawnerparams_t spawn;
        musicparams_t music;
        delayparams_t delay;
        starfieldparams_t starfield;
        waitclearparams_t waitclear;
        clearparams_t clear;
    };
} levelevent_t;

typedef struct LevelEventTimePair {
    levelevent_t event;
    shz_vec2_t pos;
} levelevent_timepair_t;

typedef enum LevelTaskType {
    LEVEL_TASK_DELAY,
    LEVEL_TASK_WAIT_CLEAR,
    LEVEL_TASK_STARFIELD_SPEED
} leveltask_type_t;

typedef struct DelayTaskParams {
    float duration;
    float accumulator;
} delaytaskparams_t;

typedef struct StarfieldSpeedTaskParams {
    float duration;
    float accumulator;
    float speed;
    bool block;
} starfieldspeedtaskparams_t;

typedef struct Level level_t;
typedef struct LevelTask {
    bool (*step)(level_t* level, struct LevelTask* task, float delta_time);
    leveltask_type_t type;
    delaytaskparams_t delay;
    starfieldspeedtaskparams_t starfield;
} leveltask_t;

#define LEVEL_MAX_TASKS 5 

typedef struct Level {
    float current_pos;
    const char* initial_music;
    float scroll_speed;

    strpool_t strpool;

    levelevent_timepair_t* events;
    int total_events;
    int current_event;
    bool is_blocked;

    leveltask_t task_queue[LEVEL_MAX_TASKS];
    int total_tasks;
} level_t;

bool level_init(level_t* level, const char* file);
void level_destroy(level_t* level);

void level_step(level_t* level, float delta_time);

bool level_finished(const level_t* level);