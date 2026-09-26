#include <stdio.h>
#include <stdint.h>
typedef struct { uint32_t s; } Rng;
static uint32_t rng_next(Rng *r)
{
    uint32_t x = r->s;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return r->s = x ? x : 1u;
}
/* Toy fight: hero wins if rng%100 < win_pct */
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static double win_rate(int win_pct, int trials, uint32_t seed)
{
    Rng r = { seed };
    int wins = 0;
    for (int i = 0; i < trials; i++)
        if ((int)(rng_next(&r) % 100u) < win_pct) wins++;
    return (double)wins / (double)trials;
}
int main(void)
{
    double a = win_rate(50, 2000, 42u);
    double b = win_rate(50, 2000, 42u);
    check(a == b, "same seed reproducible");
    check(a > 0.45 && a < 0.55, "roughly 50%");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_win_rate_toy: all checks passed");
    return 0;
}
