#include <stdio.h>

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: return the first character of the line */
static char first_of(const char *line);

int main(void)
{
    check(first_of("hello\n")=='h', "first only");
    check(first_of("t\n")=='t', "single");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_validate_loop: all checks passed");
    return 0;
}
