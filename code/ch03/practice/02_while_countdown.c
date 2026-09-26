#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: sum 3+2+1 using while (not for) */
static int countdown_sum(void)
{
    return 0;
}
int main(void)
{
    check(countdown_sum()==6, "3+2+1");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_while_countdown: all checks passed");
    return 0;
}
