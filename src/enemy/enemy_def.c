#include "enemy_def.h"
#include "../util/readutils.h"

#include "../cache/caches.h"

void enemydef_read_move_ev(file_t def_file, event_t* ev) {
    uint8_t move_type;
    fs_read(def_file, &move_type, sizeof(move_type));
    ev->move.type = move_type;

    float speed;
    fs_read(def_file, &speed, sizeof(float));
    ev->move.speed = speed;

    switch(move_type) {
    case MOVE_POINT: {
        float x, y;
        fs_read(def_file, &x, sizeof(float));
        fs_read(def_file, &y, sizeof(float));
        ev->move.target = shz_vec2_init(x, y);
        break;
    }
    case MOVE_DIRECTIONAL: {
        float angle, angle_step, duration;
        fs_read(def_file, &angle, sizeof(float));
        fs_read(def_file, &angle_step, sizeof(float));
        fs_read(def_file, &duration, sizeof(float));
        ev->move.dir.angle = angle;
        ev->move.dir.angle_step = angle_step;
        ev->move.dir.duration = duration;
        break;
    }
    case MOVE_SINE: {
        float angle, omega, amplitude, duration;
        fs_read(def_file, &angle, sizeof(float));
        fs_read(def_file, &omega, sizeof(float));
        fs_read(def_file, &amplitude, sizeof(float));
        fs_read(def_file, &duration, sizeof(float));
        ev->move.sine.angle = angle;
        ev->move.sine.omega = omega;
        ev->move.sine.amplitude = amplitude;
        ev->move.sine.duration = duration;
        break;
    }
    default:
        break;
    }
}

void enemydef_read_fire_ev(file_t def_file, event_t* ev) {
    emitter_read(&ev->fire.emitter, def_file);
    // char name_buf[256];
    // readutil_readstr(def_file, name_buf, sizeof(name_buf));
    //
    // //ev->fire.def = projdefcache_get(name_buf);
    // ev->fire.emitter.def = projdefcache_get(name_buf);
    //
    // fs_read(def_file, &ev->fire.emitter.delay, sizeof(float));
    // fs_read(def_file, &ev->fire.emitter.start_angle, sizeof(float));
    // fs_read(def_file, &ev->fire.emitter.step_angle, sizeof(float));
    //
    // uint8_t spawns_per_step;
    // fs_read(def_file, &spawns_per_step, sizeof(spawns_per_step));
    // ev->fire.emitter.spawns_per_step = spawns_per_step;
}

void enemydef_read_delay_ev(file_t def_file, event_t* ev) {
    float time;
    fs_read(def_file, &time, sizeof(time));
    ev->delay = time;
}

void enemydef_read_repeat_ev(file_t def_file, event_t* ev) {
    uint16_t count;
    fs_read(def_file, &count, sizeof(count));
    ev->repeat.count = count;

    uint16_t target;
    fs_read(def_file, &target, sizeof(target));
    ev->repeat.target = target;
}

bool enemydef_init(enemydef_t* def, const char* key) {
    
    char path[256];
    path_build_rd(path, sizeof(path), "defs/enemy", key, "dat");

    const file_t def_file = fs_open(path, O_RDONLY);

    // Read magic number
    char magic_num[4];
    fs_read(def_file, magic_num, sizeof(char) * 4);

    if(magic_num[0] != 'E' && magic_num[1] != 'D' && magic_num[2] != 'E' && magic_num[3] != 'F') {
        fs_close(def_file);
        return false;
    }

    char str_buf[256];
    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        return false;
    }
    def->anim = animcache_get(str_buf);

    // Animation clips
    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        return false;
    }
    def->clip_idle = animation_get_clip(def->anim, str_buf);

    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        return false;
    }
    def->clip_left = animation_get_clip(def->anim, str_buf);

    if(!readutil_readstr(def_file, str_buf, sizeof(str_buf))) {
        return false;
    }
    def->clip_right = animation_get_clip(def->anim, str_buf);

    // Health
    fs_read(def_file, &def->health, sizeof(uint16_t));

    // Read VM
    fs_read(def_file, &def->total_events, sizeof(def->total_events));

    for(int i = 0; i < def->total_events; ++i) {
        event_t* ev = &def->event_stack[i];
        uint8_t opcode;
        fs_read(def_file, &opcode, sizeof(opcode));

        ev->type = (eventtype_t)opcode;

        switch(ev->type) {
        case EVENT_MOVE_TO:
            enemydef_read_move_ev(def_file, ev);
            break;
        case EVENT_START_FIRING:
            enemydef_read_fire_ev(def_file, ev);
            break;
        case EVENT_DELAY:
            enemydef_read_delay_ev(def_file, ev);
            break;
        case EVENT_EXIT_SCREEN:
            fs_read(def_file, &ev->exit.speed, sizeof(float));
            break;
        case EVENT_REPEAT:
            enemydef_read_repeat_ev(def_file, ev);
            break;
        default:
            break;
        }
    }

    return true;
}

void enemydef_destroy(enemydef_t* def){

}