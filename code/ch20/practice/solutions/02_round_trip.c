#include <stdio.h>
#include <string.h>
typedef struct { int hp; int gold; } Blob;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int write_blob(const char *path, const Blob *b)
{
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "version 1\n");
    fprintf(f, "hp %d\n", b->hp);
    fprintf(f, "gold %d\n", b->gold);
    fclose(f);
    return 1;
}
static int read_blob(const char *path, Blob *b)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char key[32]; int val, ver=-1;
    Blob tmp = {0};
    while (fscanf(f, "%31s %d", key, &val) == 2) {
        if (strcmp(key, "version")==0) ver = val;
        else if (strcmp(key, "hp")==0) tmp.hp = val;
        else if (strcmp(key, "gold")==0) tmp.gold = val;
        else { fclose(f); return 0; }
    }
    fclose(f);
    if (ver != 1) return 0;
    *b = tmp;
    return 1;
}
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
