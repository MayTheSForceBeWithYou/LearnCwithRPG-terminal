/*
 * Drill 03 — sum an array by walking a pointer
 */
#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO: sum len elements starting at p (use pointer increment, not p[i] only) */
static int sum_ptr(const int *p, int len);


int main(void)
{
    int a[] = {1, 2, 3, 4};
    check(sum_ptr(a, 4) == 10, "sum 10");
    check(sum_ptr(a, 0) == 0, "empty");
    check(sum_ptr(a, 1) == 1, "one");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_walk_array: all checks passed");
    return 0;
}
