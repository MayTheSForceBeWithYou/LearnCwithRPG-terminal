#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: true when argv[1] is "--where" */
static int wants_where(int argc, char **argv);
int main(void)
{
    char *a[] = { "game", "--where" };
    char *b[] = { "game" };
    check(wants_where(2, a)==1, "flag");
    check(wants_where(1, b)==0, "none");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_argv_where: all checks passed");
    return 0;
}
