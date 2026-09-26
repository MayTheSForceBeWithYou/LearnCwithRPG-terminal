#include <stdio.h>
enum { MAX = 8 };
typedef struct { int id; int qty; } Stack;
typedef struct { Stack s[MAX]; int count; } Inv;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: remove one from index; if qty hits 0 shift later down */
static int remove_at_shift(Inv *inv, int index)
{
    (void)inv;(void)index; return 0;
}
int main(void)
{
    Inv inv = { .count = 3, .s = { {1,1},{2,1},{3,1} } };
    check(remove_at_shift(&inv, 0)==1, "rm");
    check(inv.count==2 && inv.s[0].id==2 && inv.s[1].id==3, "order kept");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_remove_shift: all checks passed");
    return 0;
}
