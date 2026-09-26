#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: clamp hp into [0, max_hp] — game logic, not the type system */
static int clamp_hp(int hp, int max_hp)
{
    (void)hp;(void)max_hp; return -999;
}
int main(void)
{
    check(clamp_hp(-5, 20)==0, "floor");
    check(clamp_hp(25, 20)==20, "ceil");
    check(clamp_hp(10, 20)==10, "ok");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_clamp_hp: all checks passed");
    return 0;
}
