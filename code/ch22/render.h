#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_PLAYER,
    TILE_NPC,
    TILE_DOOR
} TileType;

bool render_init(void);
void render_shutdown(void);
void render_clear(void);
void render_draw_tile(int x, int y, TileType t);
void render_draw_text(int x, int y, const char *s);
void render_present(void);

#endif
