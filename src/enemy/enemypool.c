#include "enemypool.h"

void enemypool_init(enemypool_t* pool, int initial_capacity) {
    pool->capacity = SHZ_MAX(8, initial_capacity);
    pool->total = 0;
    pool->enemies = malloc(sizeof(enemy_t*) * pool->capacity);
}

void enemypool_destroy(enemypool_t* pool) {

    for(int i = 0; i < pool->total; ++i) {
        enemy_t* e = pool->enemies[i];
        enemy_destroy(e);
        free(e);
    }
    pool->total = 0;
    pool->capacity = 0;

    free(pool->enemies);
}

enemy_t* enemypool_spawn(enemypool_t* pool, enemydef_t* def) {
    // Check capacity first. Resize if necessary
    if(pool->total >= pool->capacity) {
        pool->capacity *= 2;
        pool->enemies = realloc(pool->enemies, sizeof(enemy_t*) * pool->capacity);

        if(!pool->enemies) {
            arch_abort();
            return nullptr;
        }
    }

    enemy_t* enemy = malloc(sizeof(enemy_t));
    enemy_init(enemy, def, pool->total);
    
    pool->enemies[pool->total] = enemy;
    
    pool->total++;
    return enemy;
}

void enemypool_despawn(enemypool_t* pool, enemy_t* enemy) {
    enemy->is_dead = true;
}

void enemypool_clear(enemypool_t* pool) {
    for (int i = 0; i < pool->total; i++) {
        enemy_t* e = pool->enemies[i];
        e->is_dead = true;
    }
}

void enemypool_step(enemypool_t* pool, float delta_time) {

    // Step over each enemy
    for(int i = 0; i < pool->total; ++i) {
        enemy_t* e = pool->enemies[i];
        enemy_step(e, delta_time);
    }

    // Clean up the dead.
    for (int i = pool->total - 1; i >= 0; i--) {
        enemy_t* e = pool->enemies[i];
        if (e->is_dead) {
            int last_idx = pool->total - 1;
            
            // Swap last enemy into this slot
            if (i != last_idx) {
                pool->enemies[i] = pool->enemies[last_idx];
                pool->enemies[i]->pool_index = i;
            }

            enemy_destroy(e);
            free(e);
            pool->total--;
        }
    } 
}