#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int sum_to(int n)
{
    int s = 0;
    for (int i = 1; i <= n; i++) s += i;
    return s;
}
int main(void)
{
    check(sum_to(5)==15, "1..5");
    check(sum_to(0)==0, "empty");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_for_sum: all checks passed");
    return 0;
}
