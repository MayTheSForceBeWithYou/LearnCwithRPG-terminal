#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static float power(int attack, int defense)
{
    return (float)(attack + defense) / 2.0f;
}
int main(void)
{
    check(power(5, 4) > 4.4f && power(5, 4) < 4.6f, "avg 4.5");
    check(power(10, 4) > 6.9f && power(10, 4) < 7.1f, "avg 7");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_cast_avg: all checks passed");
    return 0;
}
