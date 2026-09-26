#include <stdio.h>
#include <stdlib.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    int *scores = malloc(3 * sizeof *scores);
    check(scores != NULL, "malloc");
    if (!scores) return 1;
    scores[0] = 10; scores[1] = 20; scores[2] = 30;
    check(scores[0] + scores[1] + scores[2] == 60, "sum");
    free(scores);
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_malloc_free: all checks passed");
    return 0;
}
