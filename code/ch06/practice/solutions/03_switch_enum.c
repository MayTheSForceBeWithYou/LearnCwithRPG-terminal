#include <stdio.h>
typedef enum { RANK_F, RANK_E, RANK_D, RANK_C, RANK_B, RANK_A, RANK_S, RANK_SS } Rank;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static char rank_char(Rank r)
{
    switch (r) {
        case RANK_F: return 'F';
        case RANK_E: return 'E';
        case RANK_D: return 'D';
        case RANK_C: return 'C';
        case RANK_B: return 'B';
        case RANK_A: return 'A';
        case RANK_S: return 'S';
        case RANK_SS: return 'X';
    }
    return '?';
}
int main(void)
{
    check(rank_char(RANK_S)=='S', "S");
    check(rank_char(RANK_SS)=='X', "SS");
    check(rank_char(RANK_F)=='F', "F");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_switch_enum: all checks passed");
    return 0;
}
