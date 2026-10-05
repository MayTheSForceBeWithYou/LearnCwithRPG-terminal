#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: fill dst with snprintf; return would-be length (snprintf return) */
static int fmt_into(char *dst, size_t dst_sz, const char *name, int hp);

int main(void)
{
    char buf[8];
    int n = fmt_into(buf, sizeof buf, "Elowen", 20);
    check(n > 7, "would truncate (return > size-1)");
    check(buf[sizeof buf - 1] == '\0', "always terminated");
    check(strlen(buf) < sizeof buf, "fits");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_snprintf_trunc: all checks passed");
    return 0;
}
