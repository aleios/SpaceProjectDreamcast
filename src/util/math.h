#pragma once

#include <sh4zam/shz_sh4zam.h>

SHZ_FORCE_INLINE int imin32(int a, int b) { return a < b ? a : b; }
SHZ_FORCE_INLINE int imax32(int a, int b) { return a > b ? a : b; }

SHZ_FORCE_INLINE int iclamp32(int val, int min, int max) { return imin32(imax32(val, min), max); }