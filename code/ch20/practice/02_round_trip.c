#include <stdio.h>
#include <string.h>
typedef struct { int hp; int gold; } Blob;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: write version/hp/gold text lines */
static int write_blob(const char *path, const Blob *b);
/* TODO: read them back; refuse unknown keys / bad version */
static int read_blob(const char *path, Blob *b);
int main(void)
{
    Blob a = {20, 150}, b = {0};
    check(write_blob("save_test.txt", &a)==1, "write");
    check(read_blob("save_test.txt", &b)==1, "read");
    check(b.hp==20 && b.gold==150, "match");
    remove("save_test.txt");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_round_trip: all checks passed");
    return 0;
}
