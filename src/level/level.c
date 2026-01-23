#include "level.h"

#include <adx/adx.h>

#include "../cache/caches.h"
#include "../defs/gamestate.h"
#include "../util/readutils.h"
#include "../sound/sound.h"

static bool level_task_delay(level_t* level, struct LevelTask* task, float delta_time) {

    task->delay.accumulator += delta_time;

    if(task->delay.accumulator >= task->delay.duration) {
        level->is_blocked = false;
        return true;
    }

    return false;
}

static bool level_task_wait_clear(level_t* level, struct LevelTask* task, float delta_time) {
    const enemypool_t* pool = gamestate_enemy_pool();

    if(pool->total == 0) {
        level->is_blocked = false;
        return true;
    }
    return false;
}

static bool level_task_starfield_speed(level_t* level, struct LevelTask* task, float delta_time) {
    task->starfield.accumulator += delta_time;

    float t = task->starfield.accumulator / task->starfield.duration;
    if (t > 1.0f)
        t = 1.0f;

    const float easing = shz_sinf(t * SHZ_F_PI);

    const float current_speed = shz_lerpf(STARFIELD_DEFAULT_SPEED, task->starfield.speed, easing);
    starfield_set_speed(gamestate_starfield(), current_speed);

    if (task->starfield.accumulator >= task->starfield.duration) {
        if (task->starfield.block) {
            level->is_blocked = false;
        }
        starfield_set_speed(gamestate_starfield(), STARFIELD_DEFAULT_SPEED);
        return true;
    }
    return false;
}

bool level_init(level_t* level, const char* file) {

    level->current_pos = 0.0f;
    level->events = nullptr;
    level->total_events = 0;
    level->current_event = 0;
    level->initial_music = nullptr;
    level->total_tasks = 0;
    level->is_blocked = false;

    strpool_init(&level->strpool, 4096);
    if(file) {
        char path_buf[256];
        path_build_rd(path_buf, sizeof(path_buf), "levels", file, "dat");

        const file_t level_file = fs_open(path_buf, O_RDONLY);

        if(level_file < 0) {
            printf("Failed to open level file: %s\n", path_buf);
            return false;
        }

        // Read magic num
        char magic_num[4];
        fs_read(level_file, magic_num, sizeof(char) * 4);

        if(magic_num[0] != 'L' && magic_num[1] != 'D' && magic_num[2] != 'E' && magic_num[3] != 'F') {
            printf("Invalid level file magic number: %s\n", path_buf);
            goto read_fail;
        }

        // Read initial music
        char str_buf[256];
        if (!readutil_readstr(level_file, str_buf, sizeof(str_buf))) {
            printf("Failed to read initial music from level file: %s\n", path_buf);
            goto read_fail;
        }
        level->initial_music = strpool_alloc(&level->strpool, str_buf);

        fs_read(level_file, &level->scroll_speed, sizeof(level->scroll_speed));

        // Read number of events
        uint16_t num_events;
        fs_read(level_file, &num_events, sizeof(num_events));
        level->total_events = num_events;

        level->events = malloc(sizeof(levelevent_timepair_t) * num_events);
         for(int ev_id = 0; ev_id < level->total_events; ++ev_id) {
             levelevent_timepair_t* ev = &level->events[ev_id];

             // Read position
             fs_read(level_file, &ev->pos, sizeof(ev->pos));

             // Read event type
             uint8_t opcode;
             fs_read(level_file, &opcode, sizeof(opcode));
             ev->event.type = (leveleventtype_t)opcode;

             switch(ev->event.type) {
             case LEVEL_EVENT_SPAWN: {
                 if (!readutil_readstr(level_file, str_buf, sizeof(str_buf))) {
                     printf("Failed to read spawn definition from level file: %s\n", path_buf);
                     goto read_fail;
                 }
                 ev->event.spawn.def = enemydefcache_get(str_buf);
                 break;
             }
             case LEVEL_EVENT_MUSIC: {
                 if (!readutil_readstr(level_file, str_buf, sizeof(str_buf))) {
                     goto read_fail;
                 }
                 ev->event.music.key = strpool_alloc(&level->strpool, str_buf);//strdup(str_buf);
                 fs_read(level_file, &ev->event.music.fade_in, sizeof(ev->event.music.fade_in));
                 fs_read(level_file, &ev->event.music.fade_out, sizeof(ev->event.music.fade_out));
                 break;
             }
             case LEVEL_EVENT_DELAY: {
                 fs_read(level_file, &ev->event.delay.duration, sizeof(ev->event.delay.duration));
                 break;
             }
             case LEVEL_EVENT_WAITCLEAR: {
                 fs_read(level_file, &ev->event.waitclear.timeout, sizeof(ev->event.waitclear.timeout));
                 break;
             }
             case LEVEL_EVENT_STARFIELD_SPEED: {
                 fs_read(level_file, &ev->event.starfield.speed, sizeof(ev->event.starfield.speed));
                 fs_read(level_file, &ev->event.starfield.duration, sizeof(ev->event.starfield.duration));
                 uint8_t block;
                 fs_read(level_file, &block, sizeof(block));
                 ev->event.starfield.block = block != 0;

                 //printf("Starfield speed: %f, duration: %f, block: %d\n", ev->event.starfield.speed, ev->event.starfield.duration, ev->event.starfield.block);
                 break;
             }
             case LEVEL_EVENT_CLEAR: {
                 uint8_t player_proj, enemy_proj, enemies, collectables;
                 fs_read(level_file, &player_proj, sizeof(player_proj));
                 fs_read(level_file, &enemy_proj, sizeof(enemy_proj));
                 fs_read(level_file, &enemies, sizeof(enemies));
                 fs_read(level_file, &collectables, sizeof(collectables));
                 ev->event.clear.player_projectiles = player_proj != 0;
                 ev->event.clear.enemy_projectiles = enemy_proj != 0;
                 ev->event.clear.enemies = enemies != 0;
                 ev->event.clear.collectables = collectables != 0;
                 break;
             }
             default:
                 printf("Failed to read level. Invalid opcode.\n");
                 goto read_fail;
             }
         }
        fs_close(level_file);
        return true;
read_fail:
        printf("Failed to read level file: %s\n", path_buf);
        level_destroy(level);
        fs_close(level_file);
        return false;
    }

    return true;
}

void level_destroy(level_t* level) {

    for(int i = 0; i < level->total_events; ++i) {
        levelevent_timepair_t* ev = &level->events[i];
        if(ev->event.type == LEVEL_EVENT_SPAWN) {
            enemydefcache_release(ev->event.spawn.def);
        }
    }

    free(level->events);
    strpool_destroy(&level->strpool);

    level->initial_music = nullptr;
    level->total_events = 0;
    level->current_pos = 0.0f;
    level->total_tasks = 0;
    level->is_blocked = false;
}

static leveltask_t* level_next_task(level_t* level) {
    return &level->task_queue[level->total_tasks++];
}

static void level_process_event(level_t* level, levelevent_timepair_t* pair) {
    const levelevent_t* ev = &pair->event;

    switch(ev->type) {
    case LEVEL_EVENT_SPAWN: {
        enemy_t* enemy = enemypool_spawn(gamestate_enemy_pool(), ev->spawn.def);
        enemy_set_position(enemy, shz_vec2_init(pair->pos.x, 0.0f));
        break;
    }
    case LEVEL_EVENT_MUSIC: {
        char mus_path[256];
        path_build_cd(mus_path, sizeof(mus_path), "music", ev->music.key, "adx");
        soundengine_play_mus_ex(mus_path, true, ev->music.fade_in, ev->music.fade_out);
        break;
    }
    case LEVEL_EVENT_DELAY: {
        leveltask_t* task = level_next_task(level);
        task->type = LEVEL_TASK_DELAY;
        task->delay.duration = ev->delay.duration;
        task->delay.accumulator = 0.0f;
        task->step = level_task_delay;
        level->is_blocked = true;
        return;
    }
    case LEVEL_EVENT_WAITCLEAR: {
        leveltask_t* task = level_next_task(level);
        task->type = LEVEL_TASK_WAIT_CLEAR;
        task->step = level_task_wait_clear;
        level->is_blocked = true;
        break;
    }
    case LEVEL_EVENT_STARFIELD_SPEED: {
        if (ev->starfield.duration == 0.0f) {
            starfield_set_speed(gamestate_starfield(), ev->starfield.speed);
        } else {
            leveltask_t* task = level_next_task(level);
            task->type = LEVEL_TASK_STARFIELD_SPEED;
            task->starfield.speed = ev->starfield.speed;
            task->starfield.duration = ev->starfield.duration;
            task->starfield.accumulator = 0.0f;
            task->starfield.block = ev->starfield.block;
            task->step = level_task_starfield_speed;

            level->is_blocked = ev->starfield.block;
        }
        break;
    }
    case LEVEL_EVENT_CLEAR: {
        if (ev->clear.player_projectiles) {
            projectilepool_clear(gamestate_player_projpool());
        }
        if (ev->clear.enemy_projectiles) {
            projectilepool_clear(gamestate_enemy_projpool());
        }
        if (ev->clear.enemies) {
            enemypool_clear(gamestate_enemy_pool());
        }
        if (ev->clear.collectables) {
            collectablepool_clear(gamestate_collectable_pool());
        }
        break;
    }
    default:
        break;
    }
}

void level_step(level_t* level, float delta_time) {

    // Execute tasks
    for(int tid = 0; tid < level->total_tasks;) {
        if(level->task_queue[tid].step(level, &level->task_queue[tid], delta_time)) {
            level->task_queue[tid] = level->task_queue[--level->total_tasks];
        } else {
            tid++;
        }
    }

    if (SHZ_UNLIKELY(level->is_blocked))
        return;

    // Move the 'ticker' up by 'scroll_speed' units.
    level->current_pos += level->scroll_speed * delta_time;

    for(int i = level->current_event; i < level->total_events && !level->is_blocked; ++i) {
        levelevent_timepair_t* ev = &level->events[i];

        if(ev->pos.y > level->current_pos) {
            break;
        }

        level_process_event(level, ev);

        level->current_event++;
    }
}

bool level_finished(const level_t* level) {
    return level->current_event >= level->total_events &&
           enemypool_active(gamestate_enemy_pool()) == 0;
}