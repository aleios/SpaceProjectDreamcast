#pragma once
#include "../components/sprite.h"
#include "../components/health.h"
#include "../components/collider.h"
#include "../animator.h"

#include "enemy_def.h"
#include "vm.h"
#include "../defs/projectile_def.h"

typedef struct Enemy {
    int pool_index;
    bool is_dead;
    int health;
    bool is_immune;

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