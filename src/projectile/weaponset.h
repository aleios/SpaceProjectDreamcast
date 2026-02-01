#pragma once

#include "projectile_pool.h"
#include "../defs/weaponset_def.h"

typedef struct WeaponSet {
    weaponsetdef_t def;

    bool firing;
    bool has_fired;
    int current_emitter;
} weaponset_t;

void weaponset_init(weaponset_t* set, weaponsetdef_t* def);

void weaponset_step(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time);

SHZ_FORCE_INLINE void weaponset_set_firing(weaponset_t* set, bool firing) {
    set->firing = firing;
}

SHZ_FORCE_INLINE emitter_t* weaponset_get_emitter(weaponset_t* set, int index) {
    if (!set || index < 0 || index >= WEAPONSET_MAX_EMITTERS) {
        return nullptr;
    }
    return &set->def.emitters[index];
}