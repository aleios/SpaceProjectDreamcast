#include "player.h"

#include "../gamesettings.h"
#include "../cache/caches.h"
#include "../renderer/sprite_renderer.h"

#include "../defs/gamestate.h"
#include "../globals.h"

#include "../screens/screens.h"
#include "../sound/sound.h"

#include "../screens/load_screen.h"
#include "../util/readutils.h"

typedef struct InputAccumulator {
    shz_vec2_t dir;
    bool firing;
} inputaccumulator_t;

void inputaccumulator_reset(inputaccumulator_t* acc) {
    acc->dir = shz_vec2_init(0.0f, 0.0f);
    acc->firing = false;
}

void inputaccumulator_add(inputaccumulator_t* acc, float x, float y, bool firing) {

    if(fabsf(x) >= fabsf(acc->dir.x)) {
        acc->dir.x = x;
    }
    if(fabsf(y) >= fabsf(acc->dir.y)) {
        acc->dir.y = y;
    }

    acc->firing |= firing;
}

void inputaccumulator_apply_delta(inputaccumulator_t* acc, float speed, float delta_time) {
    acc->dir = shz_vec2_normalize_safe(acc->dir);
    acc->dir = shz_vec2_scale(acc->dir, speed * delta_time);
}

static weaponset_t* player_get_weaponset(player_t* player, int index) {
    if (SHZ_UNLIKELY(index < 0 || index >= player->total_weapons)) {
        return nullptr;
    }
    return &player->weapons[index];
}

static bool player_load(player_t* player) {
    file_t f = fs_open("/rd/player.dat", O_RDONLY);

    // TODO: do a goto or we leak.
    if (f < 0) {
        return false;
    }

    // Read animation
    char path_buf[256];
    if (!readutil_readstr(f, path_buf, sizeof(path_buf))) {
        return false;
    }
    player->anim = animcache_get(path_buf);
    if (!player->anim) {
        return false;
    }

    // Read idle clip
    if (!readutil_readstr(f, path_buf, sizeof(path_buf))) {
        return false;
    }
    player->clip_idle = animation_get_clip(player->anim, path_buf);
    if (!player->clip_idle) {
        return false;
    }

    // Read left and right clips (optional fields)
    if (!readutil_readstr(f, path_buf, sizeof(path_buf))) {
        return false;
    }
    player->clip_left = animation_get_clip(player->anim, path_buf);

    if (!readutil_readstr(f, path_buf, sizeof(path_buf))) {
        return false;
    }
    player->clip_right = animation_get_clip(player->anim, path_buf);

    // Physics
    fs_read(f, &player->speed, sizeof(float));

    // Weapons
    uint16_t total_weapons;
    fs_read(f, &total_weapons, sizeof(total_weapons));
    player->total_weapons = total_weapons;

    player->weapons = malloc(sizeof(weaponset_t) * total_weapons);
    for (int weapid = 0; weapid < total_weapons; ++weapid) {
        weaponset_t* weap = &player->weapons[weapid];

        uint16_t total_emitters;
        fs_read(f, &total_emitters, sizeof(total_emitters));
        weap->total_emitters = total_emitters;

        weap->emitters = malloc(sizeof(emitter_t) * total_emitters);
        for (int emitterid = 0; emitterid < total_emitters; ++emitterid) {
            emitter_t* emitter = &weap->emitters[emitterid];
            emitter_read(emitter, f);
        }
    }

    fs_close(f);
    return true;
}

void player_init(player_t* player) {

    // Load player data
    if (!player_load(player)) {
        arch_abort();
    }

    // Setup
    transform_init(&player->transform);

    sprite_init(&player->sprite, &player->transform, player->anim->tex);

    animator_init(&player->animator, &player->sprite, player->clip_idle);

    sprite_renderer_add(&player->sprite);

    //player->speed = 0.2f;
    player->transform.origin = shz_vec2_init(24.0f, 29.0f);
    player->collider.radius = 1.0f;

    player->boom = soundengine_load_sfx("boom");
}

void player_destroy(player_t* player) {
    animation_destroy(player->anim);
    animator_destroy(&player->animator);
    sprite_renderer_remove(&player->sprite);
    sprite_destroy(&player->sprite);

    free(player->weapons);

    player->clip_idle = nullptr;
    player->clip_left = nullptr;
    player->clip_right = nullptr;
}

void player_explode(player_t* player) {

    soundengine_play_sfx(player->boom);

    g_gamestate.lives -= 1;
    if(g_gamestate.lives == 0) {
        screens_set(SCREEN_MAINMENU);
        return;
    }
    // Restart level
    g_gamestate.health = 10;
    screens_set_with_data(SCREEN_LOAD, &(loadscreen_data_t){
        .is_playlist = g_gamestate.is_playlist,
        .level = gamesettings_get_level(g_gamestate.playlist_index), // TODO: Might not be playlist...
        .playlist_index = g_gamestate.playlist_index
    });
}

SHZ_FORCE_INLINE void player_check_collision(player_t* player) {
    player->collider.center = player->transform.pos;

    projectilepool_t* projpool = gamestate_enemy_projpool();
    for(int i = 0; i < projpool->active_projectiles; ++i) {
        projectile_t* p = &projpool->projectiles[i];

        if(collider_test_circle(&player->collider, &p->collider)) {
            g_gamestate.health -= p->damage;
            if(g_gamestate.health <= 0) {
                player_explode(player);
            }
            projectilepool_release(projpool, i);
            --i;
        }
    }

    // Test collision with enemies. Insta death
    enemypool_t* enemy_pool = gamestate_enemy_pool();
    for(int i = 0; i < enemy_pool->total; ++i) {
        enemy_t* enemy = enemy_pool->enemies[i];
        if (enemy->is_dead)
            continue;

        enemy->collider.center = enemy->transform.pos;
        if(collider_test_circle(&player->collider, &enemy->collider)) {

            // Destroy enemy
            enemypool_despawn(enemy_pool, enemy);

            // Take away a life, explode, set invul frames.
            player_explode(player);

            break;
        }
    }
}

void player_step(player_t* player, float delta_time) {

    // Accumulate inputs from joypad, dpad, and keyboard.
    inputaccumulator_t acc;
    inputaccumulator_reset(&acc);

    maple_device_t* ctrl_dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(ctrl_dev) {
        const cont_state_t* state = maple_dev_status(ctrl_dev);

        const float factor = 0.007874f;
        float jx = (float)state->joyx * factor;
        float jy = (float)state->joyy * factor;

        inputaccumulator_add(&acc, jx, jy, state->a);
        inputaccumulator_add(&acc,
            (float)(state->dpad_right - state->dpad_left),
            (float)(state->dpad_down - state->dpad_up),
            false
        );
    }

    maple_device_t* kbd_dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
    if(kbd_dev) {
        kbd_state_t* state = maple_dev_status(kbd_dev);

        inputaccumulator_add(&acc,
            (float)(state->key_states[KBD_KEY_D].is_down - state->key_states[KBD_KEY_A].is_down),
            (float)(state->key_states[KBD_KEY_S].is_down - state->key_states[KBD_KEY_W].is_down),
            state->key_states[KBD_KEY_SPACE].is_down
        );
    }

    constexpr float move_threshold = 0.20f;
    if(acc.dir.x >= move_threshold) {
        animator_set_clip(&player->animator, player->clip_right);
    } else if(acc.dir.x <= -move_threshold) {
        animator_set_clip(&player->animator, player->clip_left);
    } else {
        animator_set_clip(&player->animator, player->clip_idle);
    }

    inputaccumulator_apply_delta(&acc, player->speed, delta_time);
    float new_x = player->transform.pos.x + acc.dir.x;
    float new_y = player->transform.pos.y + acc.dir.y;

    if(new_x >= 0.0f && new_x <= SCREEN_WIDTH) {
        player->transform.pos.x = new_x;
    }
    
    if(new_y >= 0.0f && new_y <= SCREEN_HEIGHT) {
        player->transform.pos.y = new_y;
    }

    player->firing = acc.firing;

    weaponset_t* set = player_get_weaponset(player, g_gamestate.current_weapon);
    int num_emitters = set ? set->total_emitters : 0;
    for (int i = 0; i < num_emitters; ++i) {
        emitter_t* emitter = weaponset_get_emitter(set, i);
        if (SHZ_UNLIKELY(!emitter)) {
            continue;
        }

        emitter->runtime.fire_timer -= delta_time;
        if (player->firing && emitter->runtime.fire_timer <= 0.0f) {
            shz_vec2_t pos = player->transform.pos;
            pos = shz_vec2_add(pos, emitter->offset);
            projectilepool_spawn(gamestate_player_projpool(), emitter, pos, emitter->start_angle);
            emitter->runtime.fire_timer = emitter->delay;
        }
    }

    animator_step(&player->animator, delta_time);

    player_check_collision(player);
}

void player_set_position(player_t* player, shz_vec2_t pos) {
    player->transform.pos = pos;
}

void player_move(player_t* player, shz_vec2_t offset) {
    player->transform.pos = shz_vec2_add(player->transform.pos, offset);
}