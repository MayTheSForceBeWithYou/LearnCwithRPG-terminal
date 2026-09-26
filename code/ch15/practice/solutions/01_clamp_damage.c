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
    check(base_damage(10, 0) == 10, "no def");
    check(base_damage(10, 100) == 1, "floor");
    check(base_damage(10, 4) == 8, "half def");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_clamp_damage: all checks passed");
    return 0;
}
