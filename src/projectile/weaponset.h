#pragma once

#include "projectile_pool.h"
#include "../defs/emitter.h"

typedef enum WeaponsetMode {
    WEAPONSET_MODE_PARALLEL = 0,  //< All emitters fire independently
    WEAPONSET_MODE_SEQUENTIAL,    //< Emitters fire in sequence once after another
    WEAPONSET_MODE_RANDOM         //< A random emitter is chosen to fire each cycle
} weaponsetmode_t;

constexpr int WEAPONSET_MAX_EMITTERS = 10;
typedef struct WeaponSet {
    weaponsetmode_t mode;
    int active_emitters;
    emitter_t emitters[WEAPONSET_MAX_EMITTERS];

    bool firing;
    bool has_fired;
    int current_emitter;
} weaponset_t;

bool weaponset_load(weaponset_t* set, file_t file);

void weaponset_step(weaponset_t* set, projectilepool_t* pool, shz_vec2_t pos, float delta_time);

SHZ_FORCE_INLINE void weaponset_set_firing(weaponset_t* set, bool firing) {
    set->firing = firing;
}

SHZ_FORCE_INLINE emitter_t* weaponset_get_emitter(weaponset_t* set, int index) {
    if (!set || index < 0 || index >= WEAPONSET_MAX_EMITTERS) {
        return nullptr;
    }
    return &set->emitters[index];
}