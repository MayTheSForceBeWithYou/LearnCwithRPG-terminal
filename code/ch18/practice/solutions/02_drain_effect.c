#include <stdio.h>
typedef struct { int hp; int max_hp; } Actor;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static void drain(Actor *caster, Actor *target, int mag)
{
    target->hp -= mag;
    caster->hp += mag / 2;
    if (caster->hp > caster->max_hp) caster->hp = caster->max_hp;
}
int main(void)
{
    Actor c = {10, 20}, t = {30, 30};
    drain(&c, &t, 10);
    check(t.hp == 20 && c.hp == 15, "drain half");
    c.hp = 18;
    drain(&c, &t, 10);
    check(c.hp == 20, "cap at max");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_drain_effect: all checks passed");
    return 0;
}
