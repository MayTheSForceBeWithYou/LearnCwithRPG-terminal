#include <stdio.h>
enum { MAX = 4 };
typedef struct { int id; int qty; } Stack;
typedef struct { Stack s[MAX]; int count; } Inv;
typedef struct { int gold; Inv inv; } Seller;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int inv_remove(Inv *inv, int id)
{
    for (int i = 0; i < inv->count; i++) {
        if (inv->s[i].id == id) {
            inv->s[i] = inv->s[inv->count - 1];
            inv->count--;
            return 1;
        }
    }
    return 0;
}
/* TODO: remove first, then pay sell_price; no pay if remove fails */
static int sell(Seller *s, int id, int sell_price);

int main(void)
{
    Seller s = { .gold = 0, .inv = { .count = 1, .s = {{5,1}} } };
    check(sell(&s, 5, 20)==1 && s.gold==20 && s.inv.count==0, "sold");
    check(sell(&s, 5, 20)==0 && s.gold==20, "missing");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_sell_commit: all checks passed");
    return 0;
}
