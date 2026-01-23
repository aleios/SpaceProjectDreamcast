#include "health.h"
#include <stdio.h>

void health_init(health_t* health, int hp, DeathCallback cb, void* data) {

    health->hp = hp;
    health->can_damage = true;
    health->death_callback = cb;
    health->data = data;
}

void health_destroy(health_t* health) {

}

void health_add(health_t* health, int value) {
    if(!health->can_damage) {
        return;
    }
    
    health->hp += value;
}