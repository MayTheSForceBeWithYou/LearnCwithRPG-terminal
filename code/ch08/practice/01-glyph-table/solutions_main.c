#include <stdio.h>
typedef enum {
    TILE_FLOOR = 0, TILE_WALL, TILE_PLAYER, TILE_WATER, TILE_COUNT
} TileType;
static char glyph_for(TileType t)
{
    static const char table[TILE_COUNT] = {
        [TILE_FLOOR] = '.', [TILE_WALL] = '#',
        [TILE_PLAYER] = '@', [TILE_WATER] = '~',
    };
    if (t < 0 || t >= TILE_COUNT) return '?';
    return table[t];
}
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(glyph_for(TILE_FLOOR) == '.', "floor");
    check(glyph_for(TILE_WALL) == '#', "wall");
    check(glyph_for(TILE_PLAYER) == '@', "player");
    check(glyph_for(TILE_WATER) == '~', "water");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01-glyph-table: all checks passed");
    return 0;
}
