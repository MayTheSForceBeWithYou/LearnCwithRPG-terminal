/*
 * Drill 03 — count non-'#' tiles in a tiny map
 */
#include <stdio.h>
enum { W = 5, H = 3 };
static const char map[H][W+1] = {
    "#####",
    "#...#",
    "#####",
};
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO */
static int count_walkable(void){ return -1; }
int main(void){
    check(count_walkable()==3, "three dots");
    if(fails){ fprintf(stderr,"%d failed\n",fails); return 1; }
    puts("03_count_walkable: all checks passed"); return 0;
}
