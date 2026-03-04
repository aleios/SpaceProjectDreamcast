#include "gamesettings.h"
#include <kos.h>

#include "util/readutils.h"
#include "cache/caches.h"

gamesettings_t g_gamesettings;

static void gamesettings_read_vmu() {
    maple_device_t* vmu = maple_enum_type(0, MAPLE_FUNC_MEMCARD);
    if (vmu) {

        uint8_t* data_buffer;
        int len;
        if (vmufs_read(vmu, "spjconf", (void*)&data_buffer, &len) != 0) {
            return;
        }

        vmu_pkg_t pkg;
        if (vmu_pkg_parse(data_buffer, len * sizeof(uint8_t), &pkg) != 0) {
            goto fail;
        }

        // Read the settings.
        if (pkg.data_len <= 0 || pkg.data == nullptr) {
            goto fail;
        }
        g_gamesettings.options = *((gameoptions_t*)pkg.data);

fail:
        free(data_buffer);
    }
}

bool gamesettings_load() {

    file_t file = fs_open("/rd/settings.dat", O_RDONLY);
    if (!file) {
        return false;
    }

    uint16_t max_lives;
    fs_read(file, &max_lives, sizeof(max_lives));
    g_gamesettings.max_lives = max_lives;

    uint16_t max_health;
    fs_read(file, &max_health, sizeof(max_health));
    g_gamesettings.max_health = max_health;

    char path_buf[256];
    readutil_readstr(file, path_buf, sizeof(path_buf));
    g_gamesettings.main_font = fontcache_get(path_buf);

    // Playlist
    strpool_init(&g_gamesettings.strpool, 8192);

    fs_read(file, &g_gamesettings.total_levels, sizeof(g_gamesettings.total_levels));
    g_gamesettings.playlist_levels = malloc(sizeof(char*) * g_gamesettings.total_levels);

    char name_buf[256];
    for (int i = 0; i < g_gamesettings.total_levels; ++i) {
        readutil_readstr(file, name_buf, sizeof(name_buf));
        g_gamesettings.playlist_levels[i] = strpool_alloc(&g_gamesettings.strpool, name_buf);
    }

    fs_close(file);

    // Load VMU settings
    g_gamesettings.options = (gameoptions_t){
        .music_volume = 255,
        .sfx_volume = 255,
        .player_collider = false,
        .enemy_collider = false,
        .projectile_collider = false
    };
    gamesettings_read_vmu();

    return true;
}

bool gamesettings_save() {

    // TODO: VMU selection screen.
    maple_device_t* vmu = maple_enum_type(0, MAPLE_FUNC_MEMCARD);
    if (!vmu)
        return false;

    vmu_pkg_t pkg = {
        .desc_short = "SPJ Config",
        .desc_long = "SPJ Config",
        .app_id = "SPJ",
        .eyecatch_type = VMUPKG_EC_NONE,
        .icon_cnt = 0,
        .icon_anim_speed = 0,
        .icon_data = nullptr,
        .data_len = sizeof(g_gamesettings.options),
        .data = (uint8_t*)&g_gamesettings.options
    };

    uint8_t* data;
    int data_len;
    if (vmu_pkg_build(&pkg, &data, &data_len) < 0) {
        goto fail;
    }

    if (vmufs_write(vmu, "spjconf", data, data_len, VMUFS_OVERWRITE) < 0) {
        goto fail;
    }

    free(data);
    return true;
fail:
    free(data);
    return false;
}

void gamesettings_destroy() {

    free(g_gamesettings.playlist_levels);
    strpool_destroy(&g_gamesettings.strpool);

    g_gamesettings.total_levels = 0;
    g_gamesettings.max_lives = 0;
    g_gamesettings.max_health = 0;
    g_gamesettings.playlist_levels = nullptr;
}