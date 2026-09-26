#include <stdio.h>
typedef struct { int ward_left; } Fighter;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: absorb up to ward_left from damage; return hp loss; reduce ward */
static int apply_hit(Fighter *f, int damage)
{
    (void)f;(void)damage; return -1;
}
int main(void)
{
    Fighter f = { .ward_left = 80 };
    check(apply_hit(&f, 30)==0 && f.ward_left==50, "full absorb");
    check(apply_hit(&f, 60)==10 && f.ward_left==0, "overflow");
    check(apply_hit(&f, 5)==5, "no ward");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_ward_pool: all checks passed");
    return 0;
}
