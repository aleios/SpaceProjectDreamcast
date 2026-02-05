#include "collectablepool.h"

#include "../gamesettings.h"
#include "../defs/gamestate.h"
#include "../renderer/render_util.h"

#include <stdio.h>
#include <string.h>

#include "../sound/sound.h"

static void collectablepool_rebind(collectable_t* c) {
    // Update internal pointers of components.
    c->sprite.transform = &c->transform;
}

void collectablepool_init(collectablepool_t* pool) {
    memset(pool->collectables, 0, sizeof(collectable_t) * MAX_COLLECTABLES);
    pool->total = 0;
}

void collectablepool_destroy(collectablepool_t* pool) {
    for (int i = 0; i < pool->total; i++) {
        collectable_destroy(&pool->collectables[i]);
    }
    pool->total = 0;
}

collectable_t* collectablepool_spawn(collectablepool_t* pool, collectabledef_t* def, shz_vec2_t position) {
    if (pool->total >= MAX_COLLECTABLES) {
        return nullptr;
    }

    collectable_t* collectable = &pool->collectables[pool->total++];

    collectable_init(collectable, def);
    collectable->transform.pos = position;

    return collectable;
}

void collectablepool_despawn(collectablepool_t* pool, int index) {
    if(!pool || index < 0 || index >= pool->total)
        return;

    if(index != pool->total - 1) {
        pool->collectables[index] = pool->collectables[pool->total - 1];

        collectable_t* c = &pool->collectables[index];
        collectablepool_rebind(c);
    }

    pool->total--;
}

void collectablepool_clear(collectablepool_t* pool) {
    pool->total = 0;
}

void collectablepool_step(collectablepool_t* pool, float delta_time) {

    player_t* player = gamestate_get_player();
    for (int i = 0; i < pool->total; i++) {
        collectable_t* c = &pool->collectables[i];

        collectable_step(c, delta_time);

        c->lifetime -= delta_time;

        if (c->lifetime <= 0.0f) {
            collectablepool_despawn(pool, i);
            i--;
            continue;
        }

        // Test collision with player
        c->collider.center = c->transform.pos;
        if (collider_test_circle(&c->collider, &player->collider)) {

            // Apply effects
            gamestate_add_health(c->health);
            gamestate_add_lives(c->lives);
            gamestate_add_weapon_power(c->weapon);

            soundengine_play_sfx(c->sfx);

            // Despawn
            collectablepool_despawn(pool, i);
            i--;
            continue;
        }

        c->transform.pos.y += c->speed * delta_time;
    }
}

void collectablepool_render(collectablepool_t* pool) {
    for (int i = 0; i < pool->total; i++) {
        collectable_t* c = &pool->collectables[i];

        // 'blink' when lifetime is below 3s
        bool display = c->lifetime >= 3000.0f || (int)c->lifetime % 200 < 150;
        if (display) {
            render_sprite(&pool->collectables[i].sprite);
        }
    }
}