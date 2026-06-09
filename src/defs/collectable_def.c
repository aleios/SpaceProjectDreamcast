#include "collectable_def.h"

#include "../cache/caches.h"
#include "../util/readutils.h"

#include "../sound/sound.h"

bool collectabledef_init(collectabledef_t *def, const char* key) {

    collectabledef_destroy(def);

    char str_buf[256];
    if (!path_build_rd(str_buf, sizeof(str_buf), "defs/collectable", key, "dat")) {
        return false;
    }

    const file_t file = fs_open(str_buf, O_RDONLY);

    if (file < 0) {
        return false;
    }

    if (!readutil_readstr(file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->anim = animcache_get(str_buf);
    if (!def->anim) {
        goto error_close;
    }

    if (!readutil_readstr(file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->clip = animation_get_clip(def->anim, str_buf);
    if (!def->clip) {
        goto error_close;
    }

    // Lifetime
    float lifetime;
    if (!READUTIL_READ_VALIDATE(file, lifetime)) {
        goto error_close;
    }
    def->lifetime = lifetime;

    // Sound effect for collection
    if (!readutil_readstr(file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->sfx = soundengine_load_sfx(str_buf);

    float collider_radius;
    if (!READUTIL_READ_VALIDATE(file, collider_radius)) {
        goto error_close;
    }
    def->collider_radius = collider_radius;

    float speed;
    if (!READUTIL_READ_VALIDATE(file, speed)) {
        goto error_close;
    }
    def->speed = speed;

    // Effects
    uint8_t health;
    if (!READUTIL_READ_VALIDATE(file, health)) {
        goto error_close;
    }
    def->health = health;

    uint8_t lives;
    if (!READUTIL_READ_VALIDATE(file, lives)) {
        goto error_close;
    }
    def->lives = lives;

    uint8_t weapon;
    if (!READUTIL_READ_VALIDATE(file, weapon)) {
        goto error_close;
    }
    def->weapon = weapon;

    int16_t score;
    if (!READUTIL_READ_VALIDATE(file, score)) {
        goto error_close;
    }
    def->score = score;

    // Load script
    uint32_t script_size;
    if (!READUTIL_READ_VALIDATE(file, script_size)) {
        goto error_close;
    }
    if (script_size > 0) {
        membuffer_t script_buffer;
        membuffer_init(&script_buffer, script_size+1);

        auto read_bytes = fs_read(file, script_buffer.data, script_size);
        if (read_bytes < 0 || read_bytes < script_size) {
            membuffer_destroy(&script_buffer);
            goto error_close;
        }
        script_buffer.data[read_bytes] = '\0';

        if (!script_init_from_memory(&def->script, &script_buffer)) {
            membuffer_destroy(&script_buffer);
            goto error_close;
        }
        membuffer_destroy(&script_buffer);
    }
    fs_close(file);
    return true;
error_close:
    collectabledef_destroy(def);
    fs_close(file);
    return false;
}

void collectabledef_destroy(collectabledef_t* def) {

    script_destroy(&def->script);

    animcache_release(def->anim);
    def->anim = nullptr;
    def->clip = nullptr;

    def->health = 0;
    def->lives = 0;
    def->weapon = 0;
}