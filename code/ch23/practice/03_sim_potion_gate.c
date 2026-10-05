#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: drink when mp==0 and potions>0 */
static int should_drink(int mp, int potions);
int main(void)
{
    check(should_drink(0, 2)==1, "drink");
    check(should_drink(5, 2)==0, "has mp");
    check(should_drink(0, 0)==0, "empty");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_sim_potion_gate: all checks passed");
    return 0;
}
