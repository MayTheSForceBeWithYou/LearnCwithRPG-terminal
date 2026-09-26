/*
 * Drill 04 — find first tile matching a char; return via out-params
 */
#include <stdio.h>
enum { W = 5, H = 3 };
static const char map[H][W+1] = {
    "#####",
    "#.@.#",
    "#####",
};
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: return 1 if found, writing *out_x,*out_y; else 0 */
static int find_tile(char needle, int *out_x, int *out_y){
    (void)needle;(void)out_x;(void)out_y; return 0;
}
int main(void){
    int x=-1,y=-1;
    check(find_tile('@', &x, &y)==1, "found");
    check(x==2 && y==1, "at 2,1");
    check(find_tile('X', &x, &y)==0, "missing");
    if(fails){ fprintf(stderr,"%d failed\n",fails); return 1; }
    puts("04_find_tile: all checks passed"); return 0;
}
