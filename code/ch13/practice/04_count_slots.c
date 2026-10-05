#include <stdio.h>
enum { MAX = 4 };
typedef struct { int used; int slots[MAX]; } Bag;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: append if used < MAX; return 1 ok / 0 full. Loop only to used. */
static int bag_add(Bag *b, int v);

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
