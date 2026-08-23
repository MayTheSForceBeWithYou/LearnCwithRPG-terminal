#ifndef MAP_H
#define MAP_H

typedef struct {
    int width;
    int height;
    char *tiles;
} Map;

/* Paired lifetime: every map_load must be matched by exactly one
   map_destroy. The caller owns the returned Map and is responsible for
   destroying it.

   Returns NULL if the file cannot be opened, is malformed, or memory
   runs out. On failure, an explanation is printed to stderr and nothing
   is left allocated. */
Map *map_load(const char *path);
void map_destroy(Map *map);

char map_tile_at(const Map *map, int x, int y);
int map_is_walkable(const Map *map, int x, int y);

#endif
