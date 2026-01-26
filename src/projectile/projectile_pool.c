#include "projectile_pool.h"
#include "../renderer/render_util.h"
#include "../globals.h"

static void projectilepool_rebind(projectile_t* p) {
    // Update internal pointers of components.
    p->sprite.transform = &p->transform;
    p->animator.sprite = &p->sprite;
    animator_refresh(&p->animator);
}

void projectilepool_init(projectilepool_t* pool, int capacity) {

    pool->projectiles = calloc(sizeof(projectile_t), capacity);
    pool->projectile_capacity = capacity;
    pool->active_projectiles = 0;

    for(int i = 0; i < capacity; ++i) {
        projectile_t* p = &pool->projectiles[i];
        p->lifetime = 0.0f;

        p->velocity = shz_vec2_init(0.0f, 0.0f);
        p->target = nullptr;
    }
}

void projectilepool_destroy(projectilepool_t* pool) {
    free(pool->projectiles);
    pool->projectile_capacity = 0;
    pool->active_projectiles = 0;
}

projectile_t* projectilepool_get(projectilepool_t* pool) {

    if(pool->active_projectiles < pool->projectile_capacity) {
        return &pool->projectiles[pool->active_projectiles];
    }
    return nullptr;
}

void projectilepool_spawn(projectilepool_t* pool, emitter_t* emitter, shz_vec2_t pos, float angle) {
    if(pool->active_projectiles >= pool->projectile_capacity || !emitter) {
        return;
    }
    projectiledef_t* def = emitter->def;

    projectile_t* p = projectilepool_get(pool);

    if(!p) {
        return;
    }

    // printf("Emitter:\nSpawns: %d\nDelay: %f\nStart Angle: %f\nStep Angle: %f\nSpeed: %f\nLifetime: %f\n",
    //     emitter->spawns_per_step, emitter->delay, emitter->start_angle, emitter->step_angle, emitter->speed, emitter->lifetime);

    p->lifetime = emitter->lifetime;

    transform_init(&p->transform);
    p->transform.pos = pos;

    sprite_init(&p->sprite, &p->transform, def->tex);

    animator_init(&p->animator, &p->sprite, def->clip);
    projectilepool_rebind(p);

    p->collider.center = p->transform.pos;
    p->collider.radius = def->collider_radius;

    p->damage = def->damage;

    // velocity set

    switch(def->target) {
    case PROJECTILETARGET_NEAREST:
        // Unimplemented
        break;
    case PROJECTILETARGET_STRONGEST:
        // Unimplemented
        break;
    default:
        p->target = nullptr;

        shz_sincos_t sc = shz_sincosf(angle);
        p->velocity.x = sc.cos;
        p->velocity.y = sc.sin;
        p->velocity = shz_vec2_scale(p->velocity, emitter->speed);

        if (def->sprite_rotates) {
            p->transform.rot = angle;
        }
        break;
    }

    pool->active_projectiles++;
}

// void projectilepool_spawn(projectilepool_t* pool, projectiledef_t* def, shz_vec2_t pos, float angle) {
//     if(pool->active_projectiles >= pool->projectile_capacity || !def) {
//         return;
//     }
//
//     projectile_t* p = projectilepool_get(pool);
//
//     if(!p) {
//         return;
//     }
//
//     p->lifetime = def->lifetime;
//
//     transform_init(&p->transform);
//     p->transform.pos = pos;
//
//     sprite_init(&p->sprite, &p->transform, def->tex);
//
//     animator_init(&p->animator, &p->sprite, def->clip);
//     projectilepool_rebind(p);
//
//     // TODO: Get from def
//     p->collider.center = p->transform.pos;
//     p->collider.radius = 5.0f;
//
//     p->damage = def->damage;
//
//     // velocity set
//
//     switch(def->target) {
//     case PROJECTILETARGET_NEAREST:
//         // Unimplemented
//         break;
//     case PROJECTILETARGET_STRONGEST:
//         // Unimplemented
//         break;
//     default:
//         p->target = nullptr;
//
//         shz_sincos_t sc = shz_sincosf(angle);
//         p->velocity.x = sc.cos;
//         p->velocity.y = sc.sin;
//         p->velocity = shz_vec2_scale(p->velocity, def->speed);
//         break;
//     }
//
//     pool->active_projectiles++;
// }

void projectilepool_release(projectilepool_t* pool, int index) {
    if(!pool || index< 0 || index >= pool->active_projectiles)
        return;

    if(index != pool->active_projectiles - 1) {
        pool->projectiles[index] = pool->projectiles[pool->active_projectiles - 1];

        projectile_t* p = &pool->projectiles[index];
        projectilepool_rebind(p);
    }

    pool->active_projectiles--;
}

void projectilepool_clear(projectilepool_t* pool) {
    pool->active_projectiles = 0;
}

void projectilepool_step(projectilepool_t* pool, float delta_time) {

    for(int i = 0; i < pool->active_projectiles; ++i) {
        projectile_t* p = &pool->projectiles[i];

        p->lifetime -= delta_time;

        // Check screen bounds and lifetime
        float dist_x = p->transform.pos.x - SCREEN_HALF_WIDTH;
        float dist_y = p->transform.pos.y - SCREEN_HALF_HEIGHT;
        shz_vec4_t screen_dist = shz_vec4_init(dist_x, dist_y, 0.0f, 0.0f);

        if(p->lifetime <= 0.0f || shz_vec4_dot(screen_dist, screen_dist) > SCREEN_BOUND_RADIUS) {
            projectilepool_release(pool, i);
            i--;
            continue;
        }

        p->transform.pos = shz_vec2_add(p->transform.pos, shz_vec2_scale(p->velocity, delta_time));
        p->collider.center = p->transform.pos;

        animator_step(&p->animator, delta_time);
    }
}

void projectilepool_render(projectilepool_t* pool) {
    for(int i = 0; i < pool->active_projectiles; ++i) {
        render_sprite(&pool->projectiles[i].sprite);
    }
}