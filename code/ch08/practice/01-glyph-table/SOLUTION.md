# Solution

```c
static char glyph_for(TileType t)
{
    static const char table[TILE_COUNT] = {
        [TILE_FLOOR]  = '.',
        [TILE_WALL]   = '#',
        [TILE_PLAYER] = '@',
        [TILE_WATER]  = '~',
    };
    if (t < 0 || t >= TILE_COUNT) return '?';
    return table[t];
}
```

Designated initializers keep the mapping obvious. A `switch` is fine too;
the point is one place owns the glyph.
