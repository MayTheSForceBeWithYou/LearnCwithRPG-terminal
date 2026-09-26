#include <stdio.h>
#include <string.h>
enum { SAVE_VERSION = 1 };
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int accept_version(const char *line)
{
    int v = -1;
    if (sscanf(line, "version %d", &v) != 1) return 0;
    return v == SAVE_VERSION;
}
int main(void)
{
    check(accept_version("version 1\n")==1, "ok");
    check(accept_version("version 2\n")==0, "bump refuse");
    check(accept_version("verson 1\n")==0, "typo");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_version_header: all checks passed");
    return 0;
}
