#include "gamesettings.h"
#include <kos.h>

#include "util/readutils.h"

gamesettings_t g_gamesettings;

static void gamesettings_read_vmu() {
    maple_device_t* vmu = maple_enum_type(0, MAPLE_FUNC_MEMCARD);
    if (vmu) {

        file_t f = fs_open("/vmu/a1/spjconf", O_META);
        if (f < 0) {
            return;
        }
        size_t len = fs_total(f);
        if (len <= 0) {
            return;
        }

        uint8_t* data_buffer = malloc(len * sizeof(uint8_t));
        fs_read(f, data_buffer, len * sizeof(uint8_t));
        vmu_pkg_t pkg;
        if (vmu_pkg_parse(data_buffer, len * sizeof(uint8_t), &pkg) != 0) {
            fs_close(f);
            free(data_buffer);
            return;
        }

        // Read the settings.
        if (pkg.data_len <= 0 || pkg.data == nullptr) {
            fs_close(f);
            free(data_buffer);
            return;
        }

        uint8_t mus_vol = pkg.data[0];
        uint8_t sfx_vol = pkg.data[1];

        g_gamesettings.options = (gameoptions_t){
            .music_volume = mus_vol,
            .sfx_volume = sfx_vol
        };

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
        .sfx_volume = 255
    };
    gamesettings_read_vmu();

    return true;
}

bool gamesettings_save() {

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

    // printf("Saving: %d\n", pkg.data_len);
    if (vmu_pkg_build(&pkg, &data, &data_len) < 0) {
        return false;
    }

    // Write package to file
    const file_t f = fs_open("/vmu/a1/spjconf", O_WRONLY | O_TRUNC | O_CREAT);
    if (f < 0) {
        free(data);
        return false;
    }

    fs_write(f, data, data_len);
    fs_close(f);

    free(data);

    return true;
}

void gamesettings_destroy() {

    free(g_gamesettings.playlist_levels);
    strpool_destroy(&g_gamesettings.strpool);

    g_gamesettings.total_levels = 0;
    g_gamesettings.max_lives = 0;
    g_gamesettings.max_health = 0;
    g_gamesettings.playlist_levels = nullptr;
}