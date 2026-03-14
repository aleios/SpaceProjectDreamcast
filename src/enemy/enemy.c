#include "enemy.h"
#include "../renderer/sprite_renderer.h"
#include "../defs/gamestate.h"
#include "../defs/direction.h"
#include "../globals.h"
#include "../cache/caches.h"
#include "../sound/sound.h"
#include "../util/randgen.h"
#include "../util/luautil.h"

// -- Lua setup --
void enemy_script_init(enemy_t* enemy, script_t* script);
void enemy_script_destroy(enemy_t* enemy);
// -- End Lua Setup --

static int enemy_get_boundary_hit(const enemy_t* enemy) {
    if (enemy->transform.pos.x - enemy->collider.radius <= 0.0f) {
        return DIRECTION_LEFT;
    }
    if (enemy->transform.pos.x + enemy->collider.radius >= SCREEN_WIDTH) {
        return DIRECTION_RIGHT;
    }
    if (enemy->transform.pos.y - enemy->collider.radius <= 0.0f) {
        return DIRECTION_UP;
    }
    if (enemy->transform.pos.y + enemy->collider.radius >= SCREEN_HEIGHT) {
        return DIRECTION_DOWN;
    }
    return DIRECTION_NONE;
}

static bool enemy_check_oob(const enemy_t* enemy) {
    const auto pos = enemy->transform.pos;
    const auto r = enemy->collider.radius;
    return (pos.y + r < 0.0f) ||
           (pos.y - r > SCREEN_HEIGHT) ||
           (pos.x + r < 0.0f) ||
           (pos.x - r > SCREEN_WIDTH);
}

void enemy_init(enemy_t* enemy, enemydef_t* def, int pool_index, shz_vec2_t initial_pos) {
    enemy->pool_index = pool_index;
    enemy->flags.raw_flags = 0x0;

    transform_init(&enemy->transform);
    enemy->transform.pos = initial_pos;

    sprite_init(&enemy->sprite, &enemy->transform, def->anim->tex);

    animator_init(&enemy->animator, &enemy->sprite, def->clip_idle);

    sprite_renderer_add(&enemy->sprite);

    // Set health.
    enemy->health = def->health;
    enemy->collider.radius = def->collision_radius;
    enemy->score = def->score;

    enemy->explode_sound = soundengine_load_sfx("boom");

    // Setup default task states
    enemy->event_sys.movement_task.active = false;

    // Setup weapon slots
    for (int i = 0; i < ENEMY_WEAPON_SLOTS; ++i) {
        if (def->weapon_slots[i] != nullptr) {
            auto slot = &enemy->event_sys.weapons[i];
            slot->valid = true;
            weaponset_init(&slot->weapon, def->weapon_slots[i]);
        } else {
            enemy->event_sys.weapons[i].valid = false;
        }
    }

    // Script init
    enemy_script_init(enemy, def->script);
}

void enemy_destroy(enemy_t* enemy) {

    // Trigger event
    lua_call_event(gamestate_lua(), enemy->event_sys.handlers.on_despawn, 0, 0);

    // Remove all hooks
    enemy_script_destroy(enemy);

    sprite_renderer_remove(&enemy->sprite);
    sprite_destroy(&enemy->sprite);
}

static shz_vec2_t move_to_point(enemy_t* enemy, enemy_movement_task_t* task, float speed) {
    const shz_vec2_t diff = shz_vec2_sub(task->target, enemy->transform.pos);
    const float dist = shz_vec2_magnitude_sqr(diff);
    if(dist <= speed*speed) {
        // Snap
        enemy->transform.pos = task->target;
        task->active = false;

        // Trigger event
        lua_call_event(gamestate_lua(), enemy->event_sys.handlers.on_target_arrive, 0, 0);
        return shz_vec2_init(0.0f, 0.0f);
    }

    const shz_vec2_t dir = shz_vec2_normalize_safe(diff);
    return shz_vec2_scale(dir, speed);
}

static shz_vec2_t move_sine(enemy_t* enemy, enemy_movement_task_t* task, float speed, float delta_time) {
    task->sine.phase += task->sine.omega * delta_time;
    const shz_sincos_t sc = shz_sincosf(task->sine.phase);
    float lateral_velocity = task->sine.amplitude * task->sine.omega * sc.cos;

    return shz_vec2_add(
        shz_vec2_scale(task->sine.fwd, speed),
        shz_vec2_scale(task->sine.perp, lateral_velocity)
    );
}

static void enemy_do_movement(enemy_t* enemy, float delta_time) {
    const auto move_task = &enemy->event_sys.movement_task;
    if (move_task->active) {

        shz_vec2_t velocity = shz_vec2_init(0.0f, 0.0f);
        const float step_speed = move_task->speed * delta_time;
        switch (move_task->type) {
        case MOVE_PLAYER_INITIAL:
        case MOVE_POINT: {
            velocity = move_to_point(enemy, move_task, step_speed);
            break;
        }
        case MOVE_DIRECTIONAL: {
            velocity = shz_vec2_scale(shz_vec2_from_angle(move_task->dir.angle), step_speed);
            break;
        }
        case MOVE_PLAYER_TARGET: {
            move_task->target = player_get_position(gamestate_get_player());
            velocity = move_to_point(enemy, move_task, step_speed);
            break;
        }
        case MOVE_SINE: {
            velocity = move_sine(enemy, move_task, step_speed, delta_time);
            break;
        }
        default:
            break;
        }

        enemy->transform.pos = shz_vec2_add(enemy->transform.pos, velocity);
    }
}

void enemy_step(enemy_t* enemy, float delta_time) {
    if(enemy->flags.dead)
        return;

    const int step_ev = enemy->event_sys.handlers.on_step;
    if (step_ev != LUA_NOREF) {
        const auto L = gamestate_lua();
        lua_pushnumber(L, delta_time);
        lua_call_event(L, step_ev, 1, 0);
    }

    // Script tasks
    enemy_do_movement(enemy, delta_time);
    for (int i = 0; i < ENEMY_WEAPON_SLOTS; ++i) {
        const auto slot = &enemy->event_sys.weapons[i];
        if (slot->valid) {
            weaponset_step(&slot->weapon, gamestate_enemy_projpool(), enemy->transform.pos, delta_time);
        }
    }

    // Update animations
    animator_step(&enemy->animator, delta_time);

    // Check playfield boundary
    const int boundary_hit = enemy_get_boundary_hit(enemy);
    const bool oob = enemy_check_oob(enemy);

    // Fire lua event when boundary is hit (if available)
    if (boundary_hit >= 0) {
        int collide_ev = enemy->event_sys.handlers.on_collide_boundary;
        if (collide_ev != LUA_NOREF) {
            const auto L = gamestate_lua();
            lua_pushnumber(L, boundary_hit);
            lua_call_event(L, collide_ev, 1, 0);
        }
    }

    // Check if initially out of bounds, but now inbounds.
    if (!enemy->flags.entered && !oob) {
        enemy->flags.entered = 1;
    }

    // If out of bounds and has already entered the playfield, then despawn it.
    if (enemy->flags.entered && oob) {
        enemypool_despawn(gamestate_enemy_pool(), enemy);
        return;
    }

    // Test against player projectiles
    enemy->collider.center = enemy->transform.pos;
    projectilepool_t* projpool = gamestate_player_projpool();
    for(int i = 0; i < projpool->active_projectiles; ++i) {
        projectile_t* p = &projpool->projectiles[i];

        if(collider_test_circle(&enemy->collider, &p->collider)) {
            enemy->health -= p->damage;

            // Damage event
            int damage_ev = enemy->event_sys.handlers.on_damage;
            if (damage_ev != LUA_NOREF) {
                const auto L = gamestate_lua();
                lua_pushinteger(L, (lua_Integer)p->damage);
                lua_call_event(L, damage_ev, 1, 0);
            }

            soundengine_play_sfx(p->hit_sound);
            projectilepool_release(projpool, i);
            --i;
        }
    }

    // Check if health is 0 or below, despawn if so.
    if (enemy->health <= 0) {

        soundengine_play_sfx(enemy->explode_sound);
        gamestate_add_score(enemy->score);

        enemypool_despawn(gamestate_enemy_pool(), enemy);
    }
}

void enemy_set_position(enemy_t* enemy, shz_vec2_t pos) {
    enemy->transform.pos = pos;
    enemy->collider.center = pos;
}