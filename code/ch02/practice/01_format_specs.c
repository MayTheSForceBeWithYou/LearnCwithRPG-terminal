#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    char buf[64];
    snprintf(buf, sizeof buf, "%d", 20);
    check(strcmp(buf, "20")==0, "%d int");
    snprintf(buf, sizeof buf, "%.1f", 7.5f);
    check(strcmp(buf, "7.5")==0, "%f float");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_format_specs: all checks passed");
    return 0;
}
