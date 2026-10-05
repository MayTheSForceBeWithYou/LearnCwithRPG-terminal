#include <stdio.h>
#include <string.h>
#include <ctype.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: accept "key = value" lines; reject garbage. Return 1 on success. */
static int parse_kv(const char *line, char *key, size_t ksz, char *val, size_t vsz);
int main(void)
{
    char k[32], v[32];
    check(parse_kv("hp = 20\n", k, sizeof k, v, sizeof v)==1, "ok");
    check(parse_kv("!!!!\n", k, sizeof k, v, sizeof v)==0, "junk");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_fuzz_tokens: all checks passed");
    return 0;
}
