#pragma once
#include "enemy.h"

typedef struct EnemyPool {
    enemy_t** enemies;
    int capacity;
    int total;
} enemypool_t;

void enemypool_init(enemypool_t* pool, int initial_capacity);
void enemypool_destroy(enemypool_t* pool);

enemy_t* enemypool_spawn(enemypool_t* pool, enemydef_t* def);
void enemypool_despawn(enemypool_t* pool, enemy_t* enemy);
void enemypool_clear(enemypool_t* pool);

void enemypool_step(enemypool_t* pool, float delta_time);

SHZ_FORCE_INLINE int enemypool_active(enemypool_t* pool) {
    return pool->total;
}