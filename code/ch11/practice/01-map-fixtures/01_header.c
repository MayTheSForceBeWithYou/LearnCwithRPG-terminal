#include <stdio.h>
#include <string.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO */
static int parse_header(const char *line, int *w, int *h)
{ (void)line;(void)w;(void)h; return 0; }
int main(void){
    int w=0,h=0;
    check(parse_header("40 20\n", &w, &h)==1 && w==40 && h==20, "ok");
    check(parse_header("nope\n", &w, &h)==0, "bad");
    check(parse_header("40\n", &w, &h)==0, "missing height");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_header: all checks passed");
    return 0;
}
