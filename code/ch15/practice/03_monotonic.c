#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int base_damage(int atk, int def)
{
    int d = atk - def / 2;
    return d < 1 ? 1 : d;
}
int main(void)
{
    int prev = base_damage(10, 0);
    for (int def = 1; def <= 200; def++) {
        int cur = base_damage(10, def);
        if (cur > prev) { check(0, "monotonic"); break; }
        prev = cur;
    }
    check(fails == 0, "never increases with def");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_monotonic: all checks passed");
    return 0;
}
