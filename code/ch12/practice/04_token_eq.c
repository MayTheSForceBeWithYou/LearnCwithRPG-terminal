#include <stdio.h>
#include <string.h>
#include <ctype.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: true if a and b have same sequence of whitespace-separated tokens */
static int tokens_equal(const char *a, const char *b);

int main(void)
{
    check(tokens_equal("one two three", "one  two   three") == 1, "extra spaces");
    check(tokens_equal("a b", "a c") == 0, "different");
    check(tokens_equal("hi", "hi") == 1, "same");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_token_eq: all checks passed");
    return 0;
}
