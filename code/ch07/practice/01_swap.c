/*
 * Drill 01 — swap via pointers
 */
#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO: exchange *a and *b */
static void swap(int *a, int *b);


int main(void)
{
    int x = 3, y = 7;
    swap(&x, &y);
    check(x == 7 && y == 3, "swapped");
    swap(&x, &x); /* no-op safe? */
    check(x == 7, "swap with self leaves value");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_swap: all checks passed");
    return 0;
}
