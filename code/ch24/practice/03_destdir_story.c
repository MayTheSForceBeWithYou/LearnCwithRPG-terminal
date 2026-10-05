#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: DESTDIR is staging only — join destdir + prefix + rel */
static void staged_path(char *dst, size_t n, const char *destdir, const char *prefix, const char *rel);
int main(void)
{
    char buf[128];
    staged_path(buf, sizeof buf, "/tmp/stage", "/usr/local", "share/game");
    check(strstr(buf, "/tmp/stage/usr/local/share/game") == buf, "staged");
    /* Runtime binary would embed /usr/local/... not /tmp/stage/... */
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_destdir_story: all checks passed");
    return 0;
}
