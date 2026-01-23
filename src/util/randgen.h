#pragma once
#include <stdlib.h>
#include <sh4zam/shz_sh4zam.h>

SHZ_FORCE_INLINE int rand_between(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

SHZ_FORCE_INLINE float rand_probablity(int chance) {
    return rand() % 100 < chance;
}