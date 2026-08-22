#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

#define MAP_WIDTH  40
#define MAP_HEIGHT 20

static const char *const world_rows[MAP_HEIGHT] = {
    "########################################",
    "#......#################...............#",
    "#......#################...#########...#",
    "#......#####........####...#.......#...#",
    "#............#######.......#.......#...#",
    "#......#####........####...#...#####...#",
    "#......#################...#...........#",
    "#......#################...#############",
    "#..........................#############",
    "############...#############..........##",
    "############...#############..........##",
    "#..............#############..........##",
    "#...####################...............#",
    "#...####################...#########...#",
    "#...................####...#########...#",
    "#########..............................#",
    "#########...####################...#####",
    "#.......................########...#####",
    "#.......................########.......#",
    "########################################"
};

Map *map_create(void)
{
    Map *map = malloc(sizeof *map);
    if (map == NULL) {
        return NULL;
    }

    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;

    map->tiles = malloc((size_t)map->width * (size_t)map->height);
    if (map->tiles == NULL) {
        /* The Map itself was allocated but its tiles were not. Release
           the Map before giving up, or we leak it. */
        free(map);
        return NULL;
    }

    for (int y = 0; y < map->height; y++) {
        memcpy(map->tiles + (size_t)y * (size_t)map->width,
               world_rows[y],
               (size_t)map->width);
    }

    return map;
}

void map_destroy(Map *map)
{
    if (map == NULL) {
        return;
    }

    free(map->tiles);
    free(map);
}

char map_tile_at(const Map *map, int x, int y)
{
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return '#';
    }

    return map->tiles[(size_t)y * (size_t)map->width + (size_t)x];
}

int map_is_walkable(const Map *map, int x, int y)
{
    return map_tile_at(map, x, y) != '#';
}
