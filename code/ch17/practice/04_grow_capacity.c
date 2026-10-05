#include <stdio.h>
#include <stdlib.h>
typedef struct { int id; int qty; } Stack;
typedef struct { Stack *s; int count; int cap; } Inv;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: grow capacity (double, from 0->4) until need fits; realloc */
static int ensure(Inv *inv, int need);
static int inv_add(Inv *inv, int id, int n)
{
    if (!ensure(inv, inv->count + 1)) return 0;
    inv->s[inv->count++] = (Stack){ id, n };
    return 1;
}
int main(void)
{
    Inv inv = {0};
    for (int i = 0; i < 6; i++) check(inv_add(&inv, i, 1)==1, "add");
    check(inv.cap >= 6 && inv.cap >= inv.count, "grew");
    free(inv.s);
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_grow_capacity: all checks passed");
    return 0;
}
