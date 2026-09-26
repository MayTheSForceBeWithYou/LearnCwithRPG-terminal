#include <stdio.h>
#include <stdint.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
typedef struct { uint32_t s; } Rng;
static uint32_t rng_next(Rng *r)
{
    uint32_t x = r->s;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return r->s = x ? x : 0xDEADBEEFu;
}
int main(void)
{
    Rng a = {12345u}, b = {12345u}, c = {999u};
    uint32_t seq[4];
    for (int i = 0; i < 4; i++) seq[i] = rng_next(&a);
    for (int i = 0; i < 4; i++) check(rng_next(&b) == seq[i], "same seed same seq");
    check(rng_next(&c) != seq[0] || rng_next(&c) != seq[1], "diff seed differs");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_xorshift_seed: all checks passed");
    return 0;
}
