#ifndef RNG_H
#define RNG_H

#include <stdint.h>

/* A tiny explicit random number generator (xorshift32).

   We carry our own instead of using rand() for three reasons: the
   sequence is identical on every machine, the whole state is one number
   we can save and replay, and nothing else in the program can disturb
   it by calling rand() somewhere we forgot about. */
typedef struct {
    uint32_t state;
} Rng;

/* A seed of 0 would make xorshift32 produce 0 forever, so it is
   silently replaced with a fixed non-zero value. */
void rng_seed(Rng *rng, uint32_t seed);

uint32_t rng_next(Rng *rng);

/* Uniform in [low, high], inclusive. Returns low if high < low. */
int rng_range(Rng *rng, int low, int high);

#endif
