#include <assert.h>
#include <stdint.h>
#include "lcg.h"

// this is the exact implementation of the needed subportion of java.utils.Random.
// for many functions i translated the code line by line.

static const uint64_t LCG_MULTIPLIER = 0x5DEECE66DULL;

static const uint64_t LCG_ADDEND = 0xBULL;

// m is 2^48. mod m is equivalent to masking with this mask:
static const uint64_t LCG_MASK = 0xFFFFFFFFFFFFULL;

void lcg_set_seed(lcg_t* rng, uint64_t seed) {
    rng->state = (seed ^ LCG_MULTIPLIER) & LCG_MASK;
}

void lcg_advance_state(lcg_t* rng) {
    rng->state = (rng->state * LCG_MULTIPLIER + LCG_ADDEND) & LCG_MASK;
}

int32_t lcg_next(lcg_t* rng, int bits) {
    lcg_advance_state(rng);
    return (int32_t)(rng->state >> (48 - bits));
}

int32_t lcg_next_int(lcg_t* rng) {
    return lcg_next(rng, 32);
}

int32_t lcg_next_int_n(lcg_t* rng, int32_t bound) {
    assert(bound > 0 && "bound is <= 0 in lcg_next_int_n");
    int32_t r = lcg_next(rng, 31);
    int32_t m = bound - 1;
    if ((bound & m) == 0) // i.e., bound is power of 2 (java.utils.Random)
        r = (int32_t) ((bound * (int64_t) r) >> 31);
    else {
        for (int u = r; u - (r = u % bound) + m < 0; u = lcg_next(rng, 31));
    }
    return r;
}

int64_t lcg_next_long(lcg_t* rng) {
    // it's okay that the bottom word remains signed. (java.utils.Random)
    return ((int64_t)lcg_next(rng, 32) << 32) + lcg_next(rng, 32);
}
