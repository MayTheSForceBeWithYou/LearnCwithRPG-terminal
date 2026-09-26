#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    char hi[] = "Hi";
    check(sizeof hi == 3, "sizeof includes NUL");
    check(strlen(hi) == 2, "strlen excludes NUL");
    check(hi[2] == '\0', "terminator present");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_strlen_sizeof: all checks passed");
    return 0;
}
