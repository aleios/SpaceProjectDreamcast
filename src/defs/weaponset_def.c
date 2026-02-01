#include "weaponset_def.h"
#include "../util/readutils.h"

bool weaponsetdef_init(weaponsetdef_t* set, const char* key) {

    char path_buf[256];
    if (!path_build_rd(path_buf, sizeof(path_buf), "defs/weapons", key, "dat")) {
        return false;
    }

    const file_t file = fs_open(path_buf, O_RDONLY);
    if (file < 0) {
        return false;
    }

    // Weapon firing mode
    uint8_t mode;
    if (!READUTIL_READ_VALIDATE(file, mode)) {
        goto error_close;
    }
    set->mode = mode;

    // Read emitters
    uint16_t total_emitters;
    if (!READUTIL_READ_VALIDATE(file, total_emitters)) {
        goto error_close;
    }
    set->active_emitters = total_emitters;

    for (int i = 0; i < total_emitters; ++i) {
        if (!emitter_read(&set->emitters[i], file)) {
            goto error_close;
        }
    }

    fs_close(file);
    return true;
error_close:
    fs_close(file);
    return false;
}

void weaponsetdef_destroy(weaponsetdef_t* set) {
    set->active_emitters = 0;
    set->mode = 0;
}