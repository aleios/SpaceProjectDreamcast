#include "projectile_def.h"
#include "../util/readutils.h"
#include "../cache/caches.h"

bool projectiledef_init(projectiledef_t* def, const char* key) {
    
    char path[256] = { 0 };
    path_build_rd(path, sizeof(path), "defs/projectile", key, "dat");

    const file_t def_file = fs_open(path, O_RDONLY);

    if(def_file <= -1) {
        return false;
    }

    // Read and test magic number
    char magic_num[4];
    fs_read(def_file, magic_num, sizeof(char) * 4);

    if(magic_num[0] != 'P' && magic_num[1] != 'D' && magic_num[2] != 'E' && magic_num[3] != 'F') {
        fs_close(def_file);
        return false;
    }

    char name_buffer[256];
    if(!readutil_readstr(def_file, name_buffer, sizeof(name_buffer))) {
        return false;
    }
    def->tex = texcache_get(name_buffer);

    if(!def->tex) {
        return false;
    }

    if(!readutil_readstr(def_file, name_buffer, sizeof(name_buffer))) {
        return false;
    }
    def->anim = animcache_get(name_buffer);

    if(!def->anim) {
        return false;
    }

    if(!readutil_readstr(def_file, name_buffer, sizeof(name_buffer))) {
        return false;
    }
    def->clip = animation_get_clip(def->anim, name_buffer);
    fs_read(def_file, &def->damage, sizeof(uint16_t));

    uint8_t rotates;
    fs_read(def_file, &rotates, sizeof(rotates));
    def->sprite_rotates = rotates > 0;

    return true;
}

void projectiledef_destroy(projectiledef_t* def) {

}