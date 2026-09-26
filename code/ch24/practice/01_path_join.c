#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static void join(char *dst, size_t n, const char *dir, const char *file)
{
    snprintf(dst, n, "%s/%s", dir, file);
}
int main(void)
{
    char buf[64];
    join(buf, sizeof buf, "/usr/local/share/game", "assets");
    check(strcmp(buf, "/usr/local/share/game/assets")==0, "join");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_path_join: all checks passed");
    return 0;
}
