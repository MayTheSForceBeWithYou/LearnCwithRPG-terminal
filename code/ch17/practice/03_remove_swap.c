#include <stdio.h>
enum { MAX = 8 };
typedef struct { int id; int qty; } Stack;
typedef struct { Stack s[MAX]; int count; } Inv;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: decrement qty; if qty hits 0, swap-remove with last slot */
static int remove_at_swap(Inv *inv, int index);
int main(void)
{
    Inv inv = { .count = 3, .s = { {1,1},{2,1},{3,1} } };
    check(remove_at_swap(&inv, 0)==1, "rm");
    check(inv.count==2, "count");
    check(inv.s[0].id==3, "last slid into hole");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_remove_swap: all checks passed");
    return 0;
}
