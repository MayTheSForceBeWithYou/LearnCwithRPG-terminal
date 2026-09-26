#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int sum_ptr(const int *p, int len)
{
    int total = 0;
    const int *end = p + len;
    while (p < end) {
        total += *p;
        p++;
    }
    return total;
}
int main(void)
{
    int a[] = {1, 2, 3, 4};
    check(sum_ptr(a, 4) == 10, "sum 10");
    check(sum_ptr(a, 0) == 0, "empty");
    check(sum_ptr(a, 1) == 1, "one");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_walk_array: all checks passed");
    return 0;
}
