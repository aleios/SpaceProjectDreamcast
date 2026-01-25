#pragma once
#include <kos.h>
#include "projectile.h"
#include "../defs/projectile_def.h"
#include "../defs/emitter.h"

// typedef struct ProjectileSpawnParams {
//     float start_angle;
//     float step_angle;
//     shz_vec2_t pos;
//     projectiledef_t* def;
// } projspawnparams_t;

typedef enum ProjectilePoolOwner {
    PROJECTILE_POOL_OWNER_PLAYER,
    PROJECTILE_POOL_OWNER_ENEMY
} projectile_pool_owner_t;

typedef struct ProjectilePool {
    projectile_t* projectiles;
    int projectile_capacity;
    int active_projectiles;

    projectile_pool_owner_t owner;
} projectilepool_t;

void projectilepool_init(projectilepool_t* pool, int capacity, projectile_pool_owner_t owner);
void projectilepool_destroy(projectilepool_t* pool);

void projectilepool_spawn(projectilepool_t* pool, emitter_t* emitter, shz_vec2_t pos, float angle);
//void projectilepool_spawn(projectilepool_t* pool, projectiledef_t* def, shz_vec2_t pos, float angle);
void projectilepool_release(projectilepool_t* pool, int index);

void projectilepool_clear(projectilepool_t* pool);

void projectilepool_step(projectilepool_t* pool, float delta_time);
void projectilepool_render(projectilepool_t* pool);