#include <stdio.h>
typedef enum { RANK_F, RANK_E, RANK_D, RANK_C, RANK_B, RANK_A, RANK_S, RANK_SS } Rank;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: map ranks to a printable char; SS -> 'X' for drill */
static char rank_char(Rank r);

int main(void)
{
    check(rank_char(RANK_S)=='S', "S");
    check(rank_char(RANK_SS)=='X', "SS");
    check(rank_char(RANK_F)=='F', "F");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_switch_enum: all checks passed");
    return 0;
}
