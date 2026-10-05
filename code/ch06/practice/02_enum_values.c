#include <stdio.h>
/* TODO: list RANK_F through RANK_SS in order so auto values match the check */
typedef enum {
} Rank;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(RANK_F==0 && RANK_S==6 && RANK_SS==7, "auto count");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_enum_values: all checks passed");
    return 0;
}
