#include "weaponset.h"

#include <stdlib.h>

bool weaponset_load(weaponset_t* set, file_t file) {
    // Emitter mode
    uint8_t mode;
    fs_read(file, &mode, sizeof(mode));
    set->mode = mode;

    uint16_t total_emitters;
    fs_read(file, &total_emitters, sizeof(total_emitters));
    set->active_emitters = total_emitters;

    for (int i = 0; i < total_emitters; ++i) {
        emitter_read(&set->emitters[i], file);
    }

    set->current_emitter = 0;
    set->firing = false;
    set->has_fired = false;

    return true;
}

static bool weaponset_update_emitter(emitter_t* emitter, projectilepool_t* pool, shz_vec2_t pos, float delta_time, bool firing) {
    emitter->runtime.fire_timer -= delta_time;
    if (!firing) {
        return false;
    }

    if (emitter->runtime.fire_timer <= 0.0f) {
        pos = shz_vec2_add(pos, emitter->offset);
        for (int i = 0; i < emitter->spawns_per_step; ++i) {
            projectilepool_spawn(pool, emitter, pos, emitter->runtime.angle);
            emitter->runtime.angle += emitter->step_angle;
        }

        constexpr float inv_tau = 0.159154943f;
        constexpr float tau     = 6.283185307f;
        emitter->runtime.angle = emitter->runtime.angle - tau * shz_floorf(emitter->runtime.angle * inv_tau);

        emitter->runtime.fire_timer = emitter->delay;
        return true;
    }
    return false;
}

static void weaponset_do_parallel(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    for (int i = 0; i < set->active_emitters; ++i) {
        weaponset_update_emitter(&set->emitters[i], pool, pos, delta_time, set->firing);
    }
}

static void weaponset_do_sequential(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    if (weaponset_update_emitter(&set->emitters[set->current_emitter], pool, pos, delta_time, set->firing)) {
        set->current_emitter = (set->current_emitter + 1) % set->active_emitters;
    }
}

static void weaponset_do_random(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    if (weaponset_update_emitter(&set->emitters[set->current_emitter], pool, pos, delta_time, set->firing)) {
        set->current_emitter = rand() % set->active_emitters;
    }
}

void weaponset_step(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {

    // Set emitter to instant when firing for the first time.
    if (SHZ_UNLIKELY(set->firing && !set->has_fired)) {
        if (set->mode == WEAPONSET_MODE_PARALLEL) {
            for (int i = 0; i < set->active_emitters; ++i) {
                set->emitters[i].runtime.fire_timer = 0.0f;
            }
        } else {
            for (int i = 0; i < set->active_emitters; ++i) {
                set->emitters[i].runtime.fire_timer =
                    (i == set->current_emitter) ? 0.0f : set->emitters[i].delay;
            }
        }

        set->has_fired = true;
    }

    switch (set->mode) {
    case WEAPONSET_MODE_PARALLEL:
        weaponset_do_parallel(set, pool, pos, delta_time);
        break;
    case WEAPONSET_MODE_SEQUENTIAL:
        weaponset_do_sequential(set, pool, pos, delta_time);
        break;
    case WEAPONSET_MODE_RANDOM:
        weaponset_do_random(set, pool, pos, delta_time);
        break;
    }
}