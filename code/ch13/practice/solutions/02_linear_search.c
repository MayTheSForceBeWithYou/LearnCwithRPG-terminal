#include <stdio.h>
typedef struct { int x, y; const char *speech; } Npc;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int npc_at(const Npc *npcs, int count, int x, int y)
{
    for (int i = 0; i < count; i++)
        if (npcs[i].x == x && npcs[i].y == y) return i;
    return -1;
}
int main(void)
{
    Npc npcs[] = { {1,2,"a"}, {5,5,"b"}, {9,1,"c"} };
    check(npc_at(npcs, 3, 5, 5) == 1, "found");
    check(npc_at(npcs, 3, 0, 0) == -1, "missing");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_linear_search: all checks passed");
    return 0;
}
