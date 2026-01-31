#pragma once
#include <kos.h>
#include <sh4zam/shz_sh4zam.h>
#include "projectile_def.h"
#include "projectile_data.h"

typedef enum ProjectileTarget {
    PROJECTILETARGET_NONE,                //< Dumb fire projectile
    PROJECTILETARGET_NEAREST,             //< Target the nearest entity
    PROJECTILETARGET_STRONGEST,           //< Target the entity with the highest total hp
} projectiletarget_t;

typedef struct Emitter {
    projectiledef_t* def; //< Projectile data
    int spawns_per_step;  //< How many projectiles to spawn per step
    float delay;          //< Time before next round of shots in ms
    float start_angle;    //< Starting angle for projectile firing
    float step_angle;     //< Angle between each projectile
    float speed;          //< Speed of the projectile in px/ms
    float lifetime;       //< Time before projectile expires in ms
    shz_vec2_t offset;    //< How far from the parent to spawn the projectiles.

    // Targeted projectile data
    projectiletarget_t target; //< Which target to lock on to.
    projectiletrackingtype_t tracking_type; //< Snapshotted or continuous tracking.
    float targeting_delay;     //< How long before the projectile 'locks on' to the target.

    struct {
        float fire_timer;  //< Accumulator
        float angle;       //< Current angle the emitter is firing at.
    } runtime;
} emitter_t;

bool emitter_read(emitter_t* emitter, file_t file);

SHZ_FORCE_INLINE void emitter_reset(emitter_t* emitter) {
    emitter->runtime.fire_timer = 0.0f;
    emitter->runtime.angle = emitter->start_angle;
}