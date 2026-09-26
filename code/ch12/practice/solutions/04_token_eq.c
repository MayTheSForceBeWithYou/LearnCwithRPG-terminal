#include <stdio.h>
#include <string.h>
#include <ctype.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static const char *skip_ws(const char *p)
{
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}
static int tokens_equal(const char *a, const char *b)
{
    a = skip_ws(a); b = skip_ws(b);
    while (*a && *b) {
        while (*a && !isspace((unsigned char)*a) && *b && !isspace((unsigned char)*b)) {
            if (*a != *b) return 0;
            a++; b++;
        }
        if ((*a && !isspace((unsigned char)*a)) || (*b && !isspace((unsigned char)*b)))
            return 0;
        a = skip_ws(a); b = skip_ws(b);
    }
    return *a == '\0' && *b == '\0';
}
int main(void)
{
    check(tokens_equal("one two three", "one  two   three") == 1, "extra spaces");
    check(tokens_equal("a b", "a c") == 0, "different");
    check(tokens_equal("hi", "hi") == 1, "same");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_token_eq: all checks passed");
    return 0;
}
