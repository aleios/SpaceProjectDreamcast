#include "projectile_pool.h"
#include "../renderer/render_util.h"
#include "../globals.h"
#include "../defs/gamestate.h"

static void projectilepool_rebind(projectile_t* p) {
    // Update internal pointers of components.
    p->sprite.transform = &p->transform;
    p->animator.sprite = &p->sprite;
    animator_refresh(&p->animator);
}

static bool get_entity_pos_by_uid(entityid_t uid, shz_vec2_t* out) {
    if (uid == ENTITY_NULL) {
        return false;
    }
    if (uid == ENTITY_PLAYER) {
        *out = gamestate_get_player()->transform.pos;
        return true;
    }

    enemypool_t* pool = gamestate_enemy_pool();

    for (int i = 0; i < pool->total; i++) {
        enemy_t* e = pool->enemies[i];
        if (!e || e->is_dead)
            continue;
        if (e->uid == uid) {
            *out = e->transform.pos;
            return true;
        }
    }
    return false;
}

static bool find_strongest_target(entityid_t* out) {
    enemypool_t* pool = gamestate_enemy_pool();
    int strongest_hp = 0;
    uint32_t strongest_uid = 0;
    for (int i = 0; i < pool->total; i++) {
        enemy_t* e = pool->enemies[i];
        if (!e || e->is_dead)
            continue;

        if (strongest_uid == 0 || e->health > strongest_hp) {
            strongest_hp = e->health;
            strongest_uid = e->uid;
        }
    }

    if (strongest_uid == 0) {
        return false;
    }
    *out = strongest_uid;
    return true;
}

static bool find_nearest_target(shz_vec2_t pos, entityid_t* out) {
    enemypool_t* pool = gamestate_enemy_pool();
    float closest_dist = 0.0f;
    uint32_t closest_uid = 0;
    for (int i = 0; i < pool->total; i++) {
        enemy_t* e = pool->enemies[i];
        if (!e || e->is_dead)
            continue;

        const float dist = shz_vec2_distance_sqr(e->transform.pos, pos);

        if (closest_uid == 0 || dist < closest_dist) {
            closest_dist = dist;
            closest_uid = e->uid;
        }
    }

    if (closest_uid == 0) {
        return false;
    }
    *out = closest_uid;
    return true;
}

void projectilepool_init(projectilepool_t* pool, int capacity, projectile_pool_owner_t owner) {

    pool->projectiles = calloc(sizeof(projectile_t), capacity);
    pool->projectile_capacity = capacity;
    pool->active_projectiles = 0;
    pool->owner = owner;

    for(int i = 0; i < capacity; ++i) {
        projectile_t* p = &pool->projectiles[i];
        p->lifetime = 0.0f;
        p->damage = 0;

        p->velocity = shz_vec2_init(0.0f, 0.0f);
        p->target_uid = 0;
        p->targeting_delay = 0.0f;
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

    p->lifetime = emitter->lifetime;

    transform_init(&p->transform);
    p->transform.pos = pos;

    sprite_init(&p->sprite, &p->transform, def->tex);
    animator_init(&p->animator, &p->sprite, def->clip);
    projectilepool_rebind(p);

    p->collider.center = p->transform.pos;
    p->collider.radius = def->collider_radius;

    p->damage = def->damage;

    // TODO: We need 2 types of tracking. Single and Continuous.
    p->speed = emitter->speed;
    p->target_uid = ENTITY_NULL;
    switch(emitter->target) {
    case PROJECTILETARGET_NEAREST: {
        if (pool->owner == PROJECTILE_POOL_OWNER_PLAYER) {
            uint32_t uid;
            if (find_nearest_target(p->transform.pos, &uid)) {
                p->target_uid = uid;
            }
        } else {
            p->target_uid = ENTITY_PLAYER;
        }
        p->targeting_delay = emitter->targeting_delay;
        break;
    }
    case PROJECTILETARGET_STRONGEST:
        if (pool->owner == PROJECTILE_POOL_OWNER_PLAYER) {
            uint32_t uid;
            if (find_strongest_target(&uid)) {
                p->target_uid = uid;
            }
        } else {
            p->target_uid = ENTITY_PLAYER;
        }
        break;
    default:
        break;
    }

    // Set initial velocity.
    shz_sincos_t sc = shz_sincosf(angle);
    p->velocity = shz_vec2_init(sc.cos, sc.sin);
    p->velocity = shz_vec2_scale(p->velocity, emitter->speed);

    p->sprite_rotates = def->sprite_rotates;
    if (p->sprite_rotates) {
        p->transform.rot = angle;
    }

    pool->active_projectiles++;
}

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

        p->targeting_delay -= delta_time;
        if(p->targeting_delay <= 0.0f) {

            shz_vec2_t target_pos;
            if (get_entity_pos_by_uid(p->target_uid, &target_pos)) {
                p->velocity = shz_vec2_normalize_safe(shz_vec2_sub(target_pos, p->transform.pos));
                p->velocity = shz_vec2_scale(p->velocity, p->speed);

                if (p->sprite_rotates) {
                    p->transform.rot = shz_atan2f(p->velocity.y, p->velocity.x);
                }

                p->target_uid = ENTITY_NULL;
            } else {
                // Lost tracking of entity.
                p->target_uid = ENTITY_NULL;
            }
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