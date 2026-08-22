#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH  40
#define MAP_HEIGHT 20

int map_is_walkable(int x, int y);
char map_tile_at(int x, int y);

#endif
