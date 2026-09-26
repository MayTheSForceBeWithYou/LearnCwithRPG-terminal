#include <stdio.h>
enum { W = 5, H = 3 };
static const char map[H][W+1] = {
    "#####",
    "#.@.#",
    "#####",
};
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int find_tile(char needle, int *out_x, int *out_y){
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (map[y][x] == needle) {
                *out_x = x;
                *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}
int main(void){
    int x=-1,y=-1;
    check(find_tile('@', &x, &y)==1, "found");
    check(x==2 && y==1, "at 2,1");
    check(find_tile('X', &x, &y)==0, "missing");
    if(fails){ fprintf(stderr,"%d failed\n",fails); return 1; }
    puts("04_find_tile: all checks passed"); return 0;
}
