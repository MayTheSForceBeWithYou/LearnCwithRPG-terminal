#include <stdio.h>
/* TODO: declare the fields the designated initializer and checks below need */
typedef struct {
} Player;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    Player p = { .x = 1, .y = 2, .gold = 50 };
    check(p.x==1 && p.y==2 && p.gold==50, "designated init");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_struct_init: all checks passed");
    return 0;
}
