#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

/* Longest line we will accept: a map row, plus a newline, plus the
   terminating NUL, plus a little slack so we can detect over-long rows
   instead of silently truncating them. */
#define MAX_LINE 1024

/* Sanity limits. A map outside these bounds is treated as malformed
   rather than trusted -- a file on disk is input, and input lies. */
#define MAX_DIMENSION 4096

static int is_valid_tile(char c)
{
    /* '#' wall, '.' floor, '+' door (walkable, and usually a warp). */
    return c == '#' || c == '.' || c == '+';
}

Map *map_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "map_load: cannot open '%s'\n", path);
        return NULL;
    }

    char line[MAX_LINE];

    if (fgets(line, sizeof line, f) == NULL) {
        fprintf(stderr, "map_load: '%s' is empty\n", path);
        fclose(f);
        return NULL;
    }

    int width = 0;
    int height = 0;
    if (sscanf(line, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "map_load: '%s' has no valid 'width height' header\n",
                path);
        fclose(f);
        return NULL;
    }

    if (width <= 0 || height <= 0 ||
        width > MAX_DIMENSION || height > MAX_DIMENSION) {
        fprintf(stderr, "map_load: '%s' has unreasonable dimensions %dx%d\n",
                path, width, height);
        fclose(f);
        return NULL;
    }

    Map *map = malloc(sizeof *map);
    if (map == NULL) {
        fprintf(stderr, "map_load: out of memory\n");
        fclose(f);
        return NULL;
    }

    map->width = width;
    map->height = height;

    map->tiles = malloc((size_t)width * (size_t)height);
    if (map->tiles == NULL) {
        fprintf(stderr, "map_load: out of memory\n");
        free(map);
        fclose(f);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        if (fgets(line, sizeof line, f) == NULL) {
            fprintf(stderr, "map_load: '%s' ended early (row %d of %d)\n",
                    path, y, height);
            map_destroy(map);
            fclose(f);
            return NULL;
        }

        /* fgets keeps the newline; trim it (and a \r, for files that
           came from Windows) before measuring the row. */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (len != (size_t)width) {
            fprintf(stderr,
                    "map_load: '%s' row %d is %zu tiles, expected %d\n",
                    path, y, len, width);
            map_destroy(map);
            fclose(f);
            return NULL;
        }

        for (int x = 0; x < width; x++) {
            if (!is_valid_tile(line[x])) {
                fprintf(stderr,
                        "map_load: '%s' row %d has invalid tile '%c'\n",
                        path, y, line[x]);
                map_destroy(map);
                fclose(f);
                return NULL;
            }
        }

        memcpy(map->tiles + (size_t)y * (size_t)width, line, (size_t)width);
    }

    fclose(f);
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
