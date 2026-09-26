#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int parse_line(const char *line, int *hp, int *gold)
{
    char key[32]; int val;
    if (sscanf(line, "%31s %d", key, &val) != 2) return 0;
    if (strcmp(key, "hp")==0) { *hp = val; return 1; }
    if (strcmp(key, "gold")==0) { *gold = val; return 1; }
    return 0; /* unknown key → refuse */
}
int main(void)
{
    int hp=0, gold=0;
    check(parse_line("hp 20\n", &hp, &gold)==1, "hp");
    check(parse_line("steps_since_encounter 7\n", &hp, &gold)==0, "unknown");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_unknown_key: all checks passed");
    return 0;
}
