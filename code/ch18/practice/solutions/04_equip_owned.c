#include <stdio.h>
enum { MAX = 8 };
typedef struct { int ids[MAX]; int count; } Owned;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int owns(const Owned *o, int id)
{
    for (int i = 0; i < o->count; i++) if (o->ids[i] == id) return 1;
    return 0;
}
static int try_equip(Owned *o, int *equipped, int id)
{
    if (!owns(o, id)) return 0;
    *equipped = id;
    return 1;
}
int main(void)
{
    Owned o = { .ids = {3,7}, .count = 2 };
    int eq = 0;
    check(try_equip(&o, &eq, 7)==1 && eq==7, "owned");
    check(try_equip(&o, &eq, 9)==0 && eq==7, "not owned");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_equip_owned: all checks passed");
    return 0;
}
