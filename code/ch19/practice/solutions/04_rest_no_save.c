#include <stdio.h>
typedef struct { int hp; int max_hp; int mp; int max_mp; } Player;
typedef struct { int saved; } Game; /* pretend */
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* Rest knows only Player — must NOT touch Game/save */
static void shop_rest(Player *p)
{
    p->hp = p->max_hp;
    p->mp = p->max_mp;
}
int main(void)
{
    Player p = {1, 20, 0, 10};
    Game g = {0};
    shop_rest(&p);
    check(p.hp==20 && p.mp==10, "restored");
    check(g.saved==0, "rest did not save");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_rest_no_save: all checks passed");
    return 0;
}
