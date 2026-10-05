#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    char hi[] = "Hi";
    /* TODO: replace each EXPECT_* with the value you predict */
    check(sizeof hi == EXPECT_SIZEOF_HI, "sizeof includes NUL");
    check(strlen(hi) == EXPECT_STRLEN_HI, "strlen excludes NUL");
    check(hi[2] == EXPECT_TERMINATOR, "terminator present");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_strlen_sizeof: all checks passed");
    return 0;
}
