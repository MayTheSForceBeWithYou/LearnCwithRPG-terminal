#include <stdio.h>
typedef enum { RANK_F, RANK_E, RANK_D, RANK_C, RANK_B, RANK_A, RANK_S, RANK_SS } Rank;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(RANK_F==0 && RANK_S==6 && RANK_SS==7, "auto count");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_enum_values: all checks passed");
    return 0;
}
