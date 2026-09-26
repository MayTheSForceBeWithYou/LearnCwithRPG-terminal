#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int row_ok(const char *line, int expect)
{
    int n = 0;
    for (const char *p = line; *p && *p != '\n'; p++) {
        if (*p != '#' && *p != '.') return 0;
        n++;
    }
    return n == expect;
}
int main(void){
    check(row_ok("###\n", 3)==1, "ok");
    check(row_ok("##\n", 3)==0, "short");
    check(row_ok("##X\n", 3)==0, "bad tile");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_row_width: all checks passed");
    return 0;
}
