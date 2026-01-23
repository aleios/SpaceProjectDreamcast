#pragma once
#include <stdint.h>

void soundengine_init();
void soundengine_cleanup();

void soundengine_play_mus_ex(const char* key, bool loop, float fade_in_time, float fade_out_time);
void soundengine_play_mus(const char* key, bool loop);
void soundengine_stop_mus();

void soundengine_pause_mus();
void soundengine_resume_mus();

uint32_t soundengine_load_sfx(const char* key);
uint32_t soundengine_get_sfx(const char* key);

bool soundengine_play_sfx(uint32_t handle);

void soundengine_step(float delta_time);