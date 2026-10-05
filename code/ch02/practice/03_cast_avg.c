#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: return (attack + defense) / 2.0f  — cast so division is float */
static float power(int attack, int defense);

int main(void)
{
    /* 5+4 = 9; /2 as int would be 4; float avg 4.5 */
    check(power(5, 4) > 4.4f && power(5, 4) < 4.6f, "avg 4.5");
    /* precedence trap: attack + defense/2 without parens is different */
    check(power(10, 4) > 6.9f && power(10, 4) < 7.1f, "avg 7");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_cast_avg: all checks passed");
    return 0;
}
