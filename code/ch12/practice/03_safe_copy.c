#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: copy src into dst[dst_sz], always NUL-terminate, never overflow */
static void safe_copy(char *dst, size_t dst_sz, const char *src);

int main(void)
{
    char buf[8];
    safe_copy(buf, sizeof buf, "short");
    check(strcmp(buf, "short") == 0, "short fits");
    safe_copy(buf, sizeof buf, "this is way too long");
    check(strlen(buf) == 7, "truncated to size-1");
    check(buf[7] == '\0', "terminated");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_safe_copy: all checks passed");
    return 0;
}
