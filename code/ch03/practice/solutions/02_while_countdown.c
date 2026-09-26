#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int countdown_sum(void)
{
    int i = 3, sum = 0;
    while (i >= 1) { sum += i; i--; }
    return sum;
}
int main(void)
{
    check(countdown_sum()==6, "3+2+1");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_while_countdown: all checks passed");
    return 0;
}
