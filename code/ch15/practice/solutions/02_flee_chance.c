#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int clamp(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
static int flee_chance(int hero_agi, int foe_agi)
{
    /* simple teaching formula: 50 + 5*(hero-foe) */
    return clamp(50 + 5 * (hero_agi - foe_agi), 1, 99);
}
int main(void)
{
    check(flee_chance(5,5)==50, "equal");
    check(flee_chance(10,5) > 50, "faster");
    check(flee_chance(1,20) >= 1 && flee_chance(1,20) <= 99, "clamped");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_flee_chance: all checks passed");
    return 0;
}
