#include "emitter.h"

#include <stdio.h>

#include "../cache/caches.h"
#include "../util/readutils.h"

bool emitter_read(emitter_t* emitter, file_t file) {

    char str_buf[256];
    if (!readutil_readstr(file, str_buf, sizeof(str_buf))) {
        return false;
    }
    emitter->def = projdefcache_get(str_buf);

    uint16_t spawns_per_step;
    fs_read(file, &spawns_per_step, sizeof(spawns_per_step));
    emitter->spawns_per_step = spawns_per_step;

    fs_read(file, &emitter->delay, sizeof(float));
    fs_read(file, &emitter->start_angle, sizeof(float));
    fs_read(file, &emitter->step_angle, sizeof(float));
    fs_read(file, &emitter->speed, sizeof(float));
    fs_read(file, &emitter->lifetime, sizeof(float));

    float x, y;
    fs_read(file, &x, sizeof(float));
    fs_read(file, &y, sizeof(float));

    emitter->offset = shz_vec2_init(x, y);

    uint8_t target;
    fs_read(file, &target, sizeof(target));
    emitter->target = target;

    if (emitter->target > PROJECTILETARGET_NONE) {
        uint8_t tracking;
        fs_read(file, &tracking, sizeof(tracking));
        emitter->tracking_type = tracking;

        float targeting_delay;
        fs_read(file, &targeting_delay, sizeof(targeting_delay));
        emitter->targeting_delay = targeting_delay;
    }

    emitter->runtime.fire_timer = emitter->delay;
    emitter->runtime.angle = emitter->start_angle;

    // printf("Emitter:\nSpawns: %d\nDelay: %f\nStart Angle: %f\nStep Angle: %f\nSpeed: %f\nLifetime: %f\nOrigin: %f, %f\n",
    //     emitter->spawns_per_step, emitter->delay, emitter->start_angle, emitter->step_angle, emitter->speed, emitter->lifetime, x, y);

    return true;
}