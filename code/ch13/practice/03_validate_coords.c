#include <stdio.h>
enum { W = 5, H = 3 };
static const char map[H][W+1] = { "#####", "#...#", "#####" };
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int walkable(int x, int y)
{
    if (x < 0 || x >= W || y < 0 || y >= H) return 0;
    return map[y][x] != '#';
}
/* TODO: return number of bad coords among npcs */
static int count_bad(int (*xy)[2], int n);

int main(void)
{
    int ok[][2] = { {1,1}, {2,1} };
    int bad[][2] = { {0,0}, {2,1} };
    check(count_bad(ok, 2) == 0, "all good");
    check(count_bad(bad, 2) == 1, "one wall");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_validate_coords: all checks passed");
    return 0;
}
