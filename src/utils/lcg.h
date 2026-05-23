#ifndef LCG_H
#define LCG_H

#include <stdint.h>

// LCG = Linear Congruential Generator
// Exact Random Algorithm, java.utils.Random uses.

typedef struct {
    uint64_t state;
} lcg_t;

/**
 * Initializes rng generator with a seed.
 */
void lcg_set_seed(lcg_t* rng, uint64_t seed);

/**
 * Generates the next set of bits.
 * Note that `bits` should never be greater then 32
 */
int32_t lcg_next(lcg_t* rng, int bits);

/**
 * Replicates Java's Random.nextInt()
 */
int32_t lcg_next_int(lcg_t* rng);

/**
 * Replicates Java's Random.nextInt(int bound)
 */
int32_t lcg_next_int_n(lcg_t* rng, int32_t bound);

/**
 * Replicates Java's Random.nextLong()
 */
int64_t lcg_next_long(lcg_t* rng);

/**
 * Generates a random double in the range [0.0, 1.0).
 */
double lcg_next_double(lcg_t* rng);

/**
 * Generates a random double in the range [min, max).
 */
double lcg_next_double_range(lcg_t* rng, double min, double max);

/**
 * Generates a random double following a normal distribution (mean 0.0, stddev 1.0).
 */
double lcg_next_gaussian(lcg_t* rng);

#endif // LCG_H
