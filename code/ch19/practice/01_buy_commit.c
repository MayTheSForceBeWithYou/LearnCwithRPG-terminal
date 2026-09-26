#include <stdio.h>
enum { MAX = 4 };
typedef struct { int id; int qty; } Stack;
typedef struct { Stack s[MAX]; int count; } Inv;
typedef struct { int gold; Inv inv; } Buyer;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int inv_add(Inv *inv, int id)
{
    if (inv->count >= MAX) return 0;
    inv->s[inv->count++] = (Stack){ id, 1 };
    return 1;
}
/* TODO: if can add AND gold>=price, add then charge; else fail with no side effects */
static int buy(Buyer *b, int id, int price)
{
    (void)b;(void)id;(void)price; return 0;
}
int main(void)
{
    Buyer b = { .gold = 100 };
    check(buy(&b, 1, 40)==1 && b.gold==60 && b.inv.count==1, "ok");
    b.gold = 10;
    check(buy(&b, 2, 40)==0 && b.gold==10 && b.inv.count==1, "too poor");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_buy_commit: all checks passed");
    return 0;
}
