#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: level 1 -> max 1; level 2-3 -> max 2; else max 3 (toy policy) */
static int max_group(int level);

int main(void)
{
    check(max_group(1)==1, "L1");
    check(max_group(2)==2, "L2");
    check(max_group(5)==3, "L5");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_group_cap: all checks passed");
    return 0;
}
