#include <stdio.h>
#include <assert.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: floor damage at 1, then assert(d >= 1) before returning */
static int base_damage(int atk, int def);
int main(void)
{
    check(base_damage(10, 100)==1, "floor");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_assert_floor: all checks passed");
    return 0;
}
