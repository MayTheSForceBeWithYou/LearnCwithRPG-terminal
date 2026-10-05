#include <stdio.h>
typedef struct { int defending; int hp; } Battle;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: apply incoming damage; if defending, half (integer), then clear flag */
static void enemy_hit(Battle *b, int damage);

static void end_round(Battle *b);

int main(void)
{
    Battle b = { .defending = 1, .hp = 20 };
    enemy_hit(&b, 10);
    check(b.hp == 15, "halved while defending");
    end_round(&b);
    check(b.defending == 0, "cleared end of round");
    enemy_hit(&b, 10);
    check(b.hp == 5, "full damage next");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_defend_flag: all checks passed");
    return 0;
}
