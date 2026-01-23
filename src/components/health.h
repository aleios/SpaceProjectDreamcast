#pragma once

#define HEALTH_INSTANT_DEAD -10000

typedef void(*DeathCallback)();

typedef struct Health {
    int hp;
    bool can_damage;
    DeathCallback death_callback;
    void* data;
} health_t;

void health_init(health_t* health, int hp, DeathCallback cb, void* data);
void health_destroy(health_t* health);
void health_add(health_t* health, int value);