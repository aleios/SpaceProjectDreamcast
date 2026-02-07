#include "weaponset.h"
#include "../sound/sound.h"
#include <stdlib.h>

void weaponset_init(weaponset_t* set, weaponsetdef_t* def) {
    if (def) {
        set->def = *def;
    }
    set->current_emitter = 0;
    set->firing = false;
    set->has_fired = false;
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

        soundengine_play_sfx(emitter->fire_sound);

        emitter->runtime.fire_timer = emitter->delay;
        return true;
    }
    return false;
}

static void weaponset_do_parallel(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    for (int i = 0; i < set->def.active_emitters; ++i) {
        weaponset_update_emitter(&set->def.emitters[i], pool, pos, delta_time, set->firing);
    }
}

static void weaponset_do_sequential(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    if (weaponset_update_emitter(&set->def.emitters[set->current_emitter], pool, pos, delta_time, set->firing)) {
        set->current_emitter = (set->current_emitter + 1) % set->def.active_emitters;
    }
}

static void weaponset_do_random(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {
    if (weaponset_update_emitter(&set->def.emitters[set->current_emitter], pool, pos, delta_time, set->firing)) {
        set->current_emitter = rand() % set->def.active_emitters;
    }
}

void weaponset_step(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time) {

    weaponsetdef_t* def = &set->def;

    // Set emitter to instant when firing for the first time.
    if (SHZ_UNLIKELY(set->firing && !set->has_fired)) {
        if (def->mode == WEAPONSET_MODE_PARALLEL) {
            for (int i = 0; i < def->active_emitters; ++i) {
                def->emitters[i].runtime.fire_timer = 0.0f;
            }
        } else {
            for (int i = 0; i < def->active_emitters; ++i) {
                def->emitters[i].runtime.fire_timer =
                    (i == set->current_emitter) ? 0.0f : def->emitters[i].delay;
            }
        }

        set->has_fired = true;
    }

    switch (def->mode) {
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