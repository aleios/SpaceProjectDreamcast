#pragma once

#include "../defs/emitter.h"

typedef struct WeaponSet {
    int total_emitters;
    emitter_t* emitters;
} weaponset_t;


SHZ_FORCE_INLINE emitter_t* weaponset_get_emitter(weaponset_t* set, int index) {
    if (!set || index < 0 || index >= set->total_emitters) {
        return nullptr;
    }
    return &set->emitters[index];
}