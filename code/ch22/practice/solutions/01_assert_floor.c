#include <stdio.h>
#include <assert.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int base_damage(int atk, int def)
{
    int d = atk - def / 2;
    if (d < 1) d = 1;
    assert(d >= 1);
    return d;
}
int main(void)
{
    check(base_damage(10, 100)==1, "floor");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_assert_floor: all checks passed");
    return 0;
}
