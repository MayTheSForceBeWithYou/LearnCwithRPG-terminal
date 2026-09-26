/*
 * TODO: complete glyph_for(). Add TILE_WATER -> '~'.
 */
#include <stdio.h>

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL,
    TILE_PLAYER,
    TILE_WATER,
    TILE_COUNT
} TileType;

/* TODO */
static char glyph_for(TileType t)
{
    (void)t;
    return '?';
}

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

int main(void)
{
    check(glyph_for(TILE_FLOOR) == '.', "floor");
    check(glyph_for(TILE_WALL) == '#', "wall");
    check(glyph_for(TILE_PLAYER) == '@', "player");
    check(glyph_for(TILE_WATER) == '~', "water");
    /* Changing wall to '%' would be a one-line table edit — do that as an
     * experiment, then change it back; do not paste into render_ncurses.c. */
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01-glyph-table: all checks passed");
    return 0;
}
