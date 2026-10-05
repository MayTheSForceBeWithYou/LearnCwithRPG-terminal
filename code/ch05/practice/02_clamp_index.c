/*
 * Drill 02 — clamp an index into [0, len)
 */
#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: if len<=0 return 0; else clamp i into [0, len-1] */
static int clamp_index(int i, int len);

int main(void){
    check(clamp_index(3, 10)==3, "inside");
    check(clamp_index(-1, 10)==0, "below");
    check(clamp_index(10, 10)==9, "at len -> last");
    check(clamp_index(99, 10)==9, "way above");
    check(clamp_index(0, 1)==0, "len 1");
    if(fails){ fprintf(stderr,"%d failed\n",fails); return 1; }
    puts("02_clamp_index: all checks passed"); return 0;
}
