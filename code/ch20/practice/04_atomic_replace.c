#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* Write tmp then rename over final — classic atomic replace on same filesystem */
static int atomic_write(const char *final_path, const char *body)
{
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.tmp", final_path);
    FILE *f = fopen(tmp, "w");
    if (!f) return 0;
    fputs(body, f);
    if (fclose(f) != 0) { remove(tmp); return 0; }
    if (rename(tmp, final_path) != 0) { remove(tmp); return 0; }
    return 1;
}
int main(void)
{
    check(atomic_write("save_test.txt", "version 1\nok\n")==1, "write");
    FILE *f = fopen("save_test.txt", "r");
    check(f != NULL, "exists");
    if (f) {
        char buf[64] = {0};
        fread(buf, 1, sizeof buf - 1, f);
        fclose(f);
        check(strstr(buf, "ok") != NULL, "content");
    }
    remove("save_test.txt");
    remove("save_test.tmp");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_atomic_replace: all checks passed");
    return 0;
}
