#include <stdio.h>
#include "map.h"

static char world[MAP_HEIGHT][MAP_WIDTH] = {
    { '#','#','#','#','#','#','#','#','#','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','.','.','#','#','#','#','.','.','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','#','#','#','#','#','#','#','#','#' }
};

void map_print(int player_x, int player_y)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            if (row == player_y && col == player_x) {
                printf("@");
            } else {
                printf("%c", world[row][col]);
            }
        }
        printf("\n");
    }
}

int map_is_walkable(int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 0;
    }

    return world[y][x] != '#';
}
