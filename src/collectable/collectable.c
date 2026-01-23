#include "collectable.h"

#include "../cache/caches.h"

void collectable_init(collectable_t* collectable, collectabletype_t type) {

    // TODO: unhardcode this crap. Make a def file for each one.
    texture_t* tex = texcache_get("ui");
    animation_t* anim = animcache_get("ui");

    collectable->type = type;

    switch (type) {
    case COLLECTABLE_HEALTH:
        collectable->clip = animation_get_clip(anim, "health");
        break;
    case COLLECTABLE_POWER:
        collectable->clip = animation_get_clip(anim, "powerup");
        break;
    case COLLECTABLE_LIFE:
        collectable->clip = animation_get_clip(anim, "life");
        break;
    default:
        break;
    }

    if (!collectable->clip) {
        return;
    }

    transform_init(&collectable->transform);
    sprite_init(&collectable->sprite, &collectable->transform, tex);

    collectable->sprite.frame = collectable->clip->frames[0].uv;
    collectable->sprite.size  = collectable->clip->frames[0].size;
    collectable->transform.origin = collectable->clip->origin;

    collectable->collider.center = collectable->transform.pos;
    collectable->collider.radius = 10.0f;
}

void collectable_destroy(collectable_t* collectable) {
    sprite_destroy(&collectable->sprite);
    collectable->lifetime = 0.0f;
}