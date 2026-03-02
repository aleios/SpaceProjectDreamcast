#include "enemy.h"
#include "../renderer/sprite_renderer.h"
#include "../defs/gamestate.h"
#include "../globals.h"
#include "../sound/sound.h"
#include "../util/randgen.h"


void enemy_init(enemy_t* enemy, enemydef_t* def, int pool_index) {
    enemy->pool_index = pool_index;
    enemy->flags.raw_flags = 0x0;

    transform_init(&enemy->transform);
    sprite_init(&enemy->sprite, &enemy->transform, def->anim->tex);

    animator_init(&enemy->animator, &enemy->sprite, def->clip_idle);

    sprite_renderer_add(&enemy->sprite);

    vm_init(&enemy->vm, enemy, def->event_stack, def->total_events);

    // Set health.
    enemy->health = def->health;
    enemy->collider.radius = def->collision_radius;
    enemy->score = def->score;

    enemy->explode_sound = soundengine_load_sfx("boom");
}

void enemy_destroy(enemy_t* enemy) {
    vm_destroy(&enemy->vm);
    sprite_renderer_remove(&enemy->sprite);
    sprite_destroy(&enemy->sprite);
}

void enemy_step(enemy_t* enemy, float delta_time) {
    if(enemy->flags.dead)
        return;

    vm_step(&enemy->vm, delta_time);

    animator_step(&enemy->animator, delta_time);

    // Check for out of bounds.
    const float dist_x = enemy->transform.pos.x - SCREEN_HALF_WIDTH;
    const float dist_y = enemy->transform.pos.y - SCREEN_HALF_HEIGHT;
    const shz_vec4_t screen_dist = shz_vec4_init(dist_x, dist_y, 0.0f, 0.0f);
    const bool oob = shz_vec4_dot(screen_dist, screen_dist) > SCREEN_BOUND_RADIUS;

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