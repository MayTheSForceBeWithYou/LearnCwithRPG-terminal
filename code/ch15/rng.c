#include "rng.h"

void rng_seed(Rng *rng, uint32_t seed)
{
    /* xorshift32 has one forbidden state: zero XORs and shifts to zero
       forever. Any non-zero constant will do as a replacement. */
    rng->state = (seed == 0) ? 0x9E3779B9u : seed;
}

uint32_t rng_next(Rng *rng)
{
    uint32_t x = rng->state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    rng->state = x;
    return x;
}

int rng_range(Rng *rng, int low, int high)
{
    if (high < low) {
        return low;
    }

    /* Work in uint32_t so the subtraction cannot overflow int, then take
       the remainder over the span. Modulo introduces a slight bias
       towards low values; for a range of a few dozen out of 2^32 the
       bias is far too small to matter in a game. */
    uint32_t span = (uint32_t)high - (uint32_t)low + 1u;
    return low + (int)(rng_next(rng) % span);
}
