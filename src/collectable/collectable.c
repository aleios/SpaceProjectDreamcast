#include "collectable.h"

#include "../cache/caches.h"

void collectable_init(collectable_t* collectable, collectabledef_t* def) {

    transform_init(&collectable->transform);
    sprite_init(&collectable->sprite, &collectable->transform, def->anim->tex);

    collectable->clip = def->clip;

    animator_init(&collectable->animator, &collectable->sprite, collectable->clip);

    collectable->collider.center = collectable->transform.pos;
    collectable->collider.radius = def->collider_radius;

    collectable->lifetime = def->lifetime;

    collectable->health = def->health;
    collectable->lives = def->lives;
    collectable->weapon = def->weapon;
    collectable->score = def->score;
    collectable->sfx = def->sfx;

    collectable->speed = def->speed;

    collectable->script = &def->script;
}

void collectable_destroy(collectable_t* collectable) {
    sprite_destroy(&collectable->sprite);
    collectable->lifetime = 0.0f;
}

void collectable_step(collectable_t* collectable, float delta_time) {
    animator_step(&collectable->animator, delta_time);
}