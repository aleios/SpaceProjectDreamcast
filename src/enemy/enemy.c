#include "enemy.h"
#include "../renderer/sprite_renderer.h"
#include "../defs/gamestate.h"
#include "../globals.h"
#include "../sound/sound.h"
#include "../util/randgen.h"


void enemy_init(enemy_t* enemy, enemydef_t* def, int pool_index) {
    enemy->pool_index = pool_index;
    enemy->is_dead = false;

    transform_init(&enemy->transform);
    sprite_init(&enemy->sprite, &enemy->transform, def->anim->tex);

    animator_init(&enemy->animator, &enemy->sprite, def->clip_idle);

    sprite_renderer_add(&enemy->sprite);

    vm_init(&enemy->vm, enemy, def->event_stack, def->total_events);

    // Set health.
    enemy->health = def->health;

    // TODO: Determine based on def.
    enemy->collider.radius = 20.0f;

    enemy->explode_sound = soundengine_load_sfx("boom");
}

void enemy_destroy(enemy_t* enemy) {
    vm_destroy(&enemy->vm);
    sprite_renderer_remove(&enemy->sprite);
    sprite_destroy(&enemy->sprite);
}

void enemy_step(enemy_t* enemy, float delta_time) {
    if(enemy->is_dead)
        return;

    vm_step(&enemy->vm, delta_time);

    animator_step(&enemy->animator, delta_time);

    // TODO: Take into account sprite size.
    if(enemy->transform.pos.x <= -20.0f || enemy->transform.pos.x >= SCREEN_WIDTH + 20.0f) {
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
            projectilepool_release(projpool, i);
            --i;
        }
    }

    // Check if health is 0 or below, despawn if so.
    if (enemy->health <= 0) {

        soundengine_play_sfx(enemy->explode_sound);

        // TODO: Drop chance from def
        if (rand_probablity(60)) {
            int type = rand_between(0, COLLECTABLE_TYPE_COUNT-1);
            collectablepool_spawn(gamestate_collectable_pool(), type, enemy->transform.pos);
        }
        enemypool_despawn(gamestate_enemy_pool(), enemy);
    }
}

void enemy_set_position(enemy_t* enemy, shz_vec2_t pos) {
    enemy->transform.pos = pos;
    enemy->collider.center = pos;
}