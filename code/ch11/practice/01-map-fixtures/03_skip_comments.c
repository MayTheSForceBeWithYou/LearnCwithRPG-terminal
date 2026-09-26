#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO */
static int count_data_lines(const char *text)
{ (void)text; return -1; }
int main(void){
    const char *sample = "; hdr\n3 2\n; mid\n###\n#.#\n";
    check(count_data_lines(sample)==3, "header+2 rows");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_skip_comments: all checks passed");
    return 0;
}
