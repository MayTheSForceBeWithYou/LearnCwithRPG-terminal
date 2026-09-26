#include <stdio.h>
typedef struct { int xp_reward; int gold_reward; int fled; int won; } Battle;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: on kill add xp/gold; finish returns xp to grant (full if won, accumulated if fled) */
static void on_kill(Battle *b, int xp, int gold)
{
    (void)b;(void)xp;(void)gold;
}
static int xp_on_finish(const Battle *b)
{
    (void)b; return -1;
}
int main(void)
{
    Battle b = {0};
    on_kill(&b, 10, 3);
    on_kill(&b, 5, 1);
    b.fled = 1;
    check(xp_on_finish(&b) == 15, "partial on flee");
    b.fled = 0; b.won = 1;
    check(xp_on_finish(&b) == 15, "same pool on win");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_reward_accum: all checks passed");
    return 0;
}
