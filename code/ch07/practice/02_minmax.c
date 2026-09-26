/*
 * Drill 02 — two out-parameters
 */
#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO: write the smaller value to *out_min, larger to *out_max */
static void minmax(int a, int b, int *out_min, int *out_max)
{
    (void)a; (void)b; (void)out_min; (void)out_max;
}

int main(void)
{
    int lo = 0, hi = 0;
    minmax(5, 2, &lo, &hi);
    check(lo == 2 && hi == 5, "unordered inputs");
    minmax(4, 4, &lo, &hi);
    check(lo == 4 && hi == 4, "equal");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_minmax: all checks passed");
    return 0;
}
