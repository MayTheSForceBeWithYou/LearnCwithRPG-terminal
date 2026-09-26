#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(15 / 20 == 0, "int truncates toward zero");
    check((float)15 / 20 > 0.7f && (float)15 / 20 < 0.8f, "cast restores fraction");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_int_div: all checks passed");
    return 0;
}
