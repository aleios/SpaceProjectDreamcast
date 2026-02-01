#pragma once
#include "emitter.h"

typedef enum WeaponsetMode {
    WEAPONSET_MODE_PARALLEL = 0,  //< All emitters fire independently
    WEAPONSET_MODE_SEQUENTIAL,    //< Emitters fire in sequence once after another
    WEAPONSET_MODE_RANDOM         //< A random emitter is chosen to fire each cycle
} weaponsetmode_t;

constexpr int WEAPONSET_MAX_EMITTERS = 10;
typedef struct WeaponSetDef {
    weaponsetmode_t mode;
    int active_emitters;
    emitter_t emitters[WEAPONSET_MAX_EMITTERS];
} weaponsetdef_t;

bool weaponsetdef_init(weaponsetdef_t* set, const char* key);
void weaponsetdef_destroy(weaponsetdef_t* set);