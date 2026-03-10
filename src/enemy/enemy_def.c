#include "enemy_def.h"
#include "../util/readutils.h"

#include "../cache/caches.h"

bool enemydef_init(enemydef_t* def, const char* key) {
    
    char path[256];
    path_build_rd(path, sizeof(path), "defs/enemy", key, "dat");

    const file_t def_file = fs_open(path, O_RDONLY);

    if (def_file < 0) {
        return false;
    }

    // Read magic number
    char magic_num[4];
    if (!READUTIL_READ_VALIDATE_EX(def_file, magic_num, sizeof(char) * 4)) {
        goto error_close;
    }

    if(magic_num[0] != 'E' && magic_num[1] != 'D' && magic_num[2] != 'E' && magic_num[3] != 'F') {
        goto error_close;
    }

    char str_buf[256];
    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->anim = animcache_get(str_buf);

    // Animation clips
    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->clip_idle = animation_get_clip(def->anim, str_buf);

    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->clip_left = animation_get_clip(def->anim, str_buf);

    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->clip_right = animation_get_clip(def->anim, str_buf);

    // Health
    if (!READUTIL_READ_VALIDATE(def_file, def->health)) {
        goto error_close;
    }

    // Collider radius
    if (!READUTIL_READ_VALIDATE(def_file, def->collision_radius)) {
        goto error_close;
    }

    // Score
    int16_t score;
    if (!READUTIL_READ_VALIDATE(def_file, score)) {
        goto error_close;
    }
    def->score = score;

    // Script
    if (!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        goto error_close;
    }
    def->script = scriptcache_get(str_buf);

    // Weapon slots
    for (int i = 0; i < ENEMY_WEAPON_SLOTS; ++i) {
        if (!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
            goto error_close;
        }

        if (str_valid(str_buf)) {
            def->weapon_slots[i] = weaponsetcache_get(str_buf);
        } else {
            def->weapon_slots[i] = nullptr;
        }
    }

    fs_close(def_file);
    return true;
error_close:
    fs_close(def_file);
    return false;
}

void enemydef_destroy(enemydef_t* def){

}