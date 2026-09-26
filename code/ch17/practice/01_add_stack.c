#include <stdio.h>
enum { MAX = 8 };
typedef struct { int id; int qty; } Stack;
typedef struct { Stack s[MAX]; int count; } Inv;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: if id exists, qty+=n; else new stack if room; return 1/0 */
static int inv_add(Inv *inv, int id, int n)
{
    (void)inv;(void)id;(void)n; return 0;
}
int main(void)
{
    Inv inv = {0};
    check(inv_add(&inv, 1, 2)==1 && inv.count==1 && inv.s[0].qty==2, "new");
    check(inv_add(&inv, 1, 3)==1 && inv.count==1 && inv.s[0].qty==5, "stack");
    check(inv_add(&inv, 2, 1)==1 && inv.count==2, "second");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_add_stack: all checks passed");
    return 0;
}
