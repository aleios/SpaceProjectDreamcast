#pragma once
#include "../components/sprite.h"
#include "../components/collider.h"
#include "../animator.h"
#include "enemy_def.h"
#include "vm.h"
#include "../entityid.h"

typedef union EnemyFlags {
    struct {
        uint8_t dead    : 1;
        uint8_t immune  : 1;
        uint8_t entered : 1;
    };
    uint8_t raw_flags;
} enemyflags_t;

typedef struct Enemy {
    int pool_index;
    entityid_t uid;
    int health;
    int score;

    enemyflags_t flags;

    transform_t transform;
    sprite_t sprite;
    circlecollider_t collider;

    animator_t animator;
    animationclip_t* idle_clip;
    animationclip_t* left_clip;
    animationclip_t* right_clip;

    virtualmachine_t vm;

    uint32_t explode_sound;
} enemy_t;

void enemy_init(enemy_t* enemy, enemydef_t* def, int pool_index);
void enemy_destroy(enemy_t* enemy);

void enemy_step(enemy_t* enemy, float delta_time);

void enemy_set_position(enemy_t* enemy, shz_vec2_t pos);

SHZ_FORCE_INLINE void enemy_set_dead(enemy_t* enemy) {
    enemy->flags.dead = 1;
}

SHZ_FORCE_INLINE bool enemy_is_dead(enemy_t* enemy) {
    return enemy->flags.dead;
}

SHZ_FORCE_INLINE void enemy_set_immune(enemy_t* enemy, bool immune) {
    enemy->flags.immune = immune;
}

SHZ_FORCE_INLINE bool enemy_is_immune(enemy_t* enemy) {
    return enemy->flags.immune;
}