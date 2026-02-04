#include "collectable.h"

#include "../cache/caches.h"

void collectable_init(collectable_t* collectable, collectabledef_t* def) {

    transform_init(&collectable->transform);
    sprite_init(&collectable->sprite, &collectable->transform, def->anim->tex);

    // TODO: Animator
    collectable->clip = def->clip;

    collectable->sprite.frame = collectable->clip->frames[0].uv;
    collectable->sprite.size  = collectable->clip->frames[0].size;
    collectable->transform.origin = collectable->clip->origin;

    collectable->collider.center = collectable->transform.pos;
    collectable->collider.radius = def->collider_radius;

    collectable->lifetime = def->lifetime;

    collectable->health = def->health;
    collectable->lives = def->lives;
    collectable->weapon = def->weapon;
    collectable->sfx = def->sfx;
}

void collectable_destroy(collectable_t* collectable) {
    sprite_destroy(&collectable->sprite);
    collectable->lifetime = 0.0f;
}