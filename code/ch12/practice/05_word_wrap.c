#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
enum { MAX_LINES = 8, LINE_LEN = 16 };
/* TODO: wrap words into lines[][]; return line count; never exceed width */
static int wrap(const char *text, int width, char lines[MAX_LINES][LINE_LEN]);

int main(void)
{
    char lines[MAX_LINES][LINE_LEN];
    int n = wrap("hello brave world", 10, lines);
    check(n >= 2, "needs at least two lines");
    check(strlen(lines[0]) <= 10, "width");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("05_word_wrap: all checks passed");
    return 0;
}
