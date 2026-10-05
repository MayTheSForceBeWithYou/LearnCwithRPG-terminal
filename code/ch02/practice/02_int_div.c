#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    /* TODO: an expression that is true because int division truncates 15/20 */
    check(INT_DIV_PROOF, "int truncates toward zero");
    /* TODO: an expression that is true because a cast restores the fraction of 15/20 */
    check(FLOAT_DIV_PROOF, "cast restores fraction");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_int_div: all checks passed");
    return 0;
}
