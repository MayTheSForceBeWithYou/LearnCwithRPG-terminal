#include <stdio.h>
enum { MAX = 4 };
typedef struct { int used; int slots[MAX]; } Bag;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int bag_add(Bag *b, int v)
{
    if (b->used >= MAX) return 0;
    b->slots[b->used++] = v;
    return 1;
}
int main(void)
{
    Bag b = {0};
    check(bag_add(&b, 10)==1 && b.used==1, "add");
    check(bag_add(&b, 20)==1, "add2");
    check(bag_add(&b, 30)==1, "add3");
    check(bag_add(&b, 40)==1, "add4");
    check(bag_add(&b, 50)==0 && b.used==4, "full");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_count_slots: all checks passed");
    return 0;
}
