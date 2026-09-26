#include <stdio.h>
#include <string.h>
#include <ctype.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
enum { MAX_LINES = 8, LINE_LEN = 16 };
static int wrap(const char *text, int width, char lines[MAX_LINES][LINE_LEN])
{
    int line = 0, col = 0;
    if (width <= 0 || width >= LINE_LEN) width = LINE_LEN - 1;
    lines[0][0] = '\0';
    while (*text && line < MAX_LINES) {
        while (*text && isspace((unsigned char)*text)) text++;
        if (!*text) break;
        const char *w = text;
        while (*text && !isspace((unsigned char)*text)) text++;
        int wlen = (int)(text - w);
        if (wlen >= LINE_LEN) wlen = LINE_LEN - 1;
        if (col > 0 && col + 1 + wlen > width) {
            lines[line][col] = '\0';
            line++; col = 0;
            if (line >= MAX_LINES) break;
        }
        if (col > 0) lines[line][col++] = ' ';
        memcpy(lines[line] + col, w, (size_t)wlen);
        col += wlen;
        lines[line][col] = '\0';
    }
    if (col > 0 || line == 0) {
        lines[line][col] = '\0';
        line++;
    }
    return line;
}
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
