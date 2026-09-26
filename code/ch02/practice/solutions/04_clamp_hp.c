#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int clamp_hp(int hp, int max_hp)
{
    if (hp < 0) return 0;
    if (hp > max_hp) return max_hp;
    return hp;
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
