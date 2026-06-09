#pragma once
#include "../util/membuffer.h"

typedef struct Script {
    membuffer_t data;
} script_t;

bool script_init_from_memory(script_t* script, membuffer_t* memory);
bool script_init(script_t* script, const char* key);
void script_destroy(script_t* script);

bool script_load(script_t* script);