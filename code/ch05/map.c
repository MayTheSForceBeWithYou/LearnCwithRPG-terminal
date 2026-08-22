#include <stdio.h>
#include "map.h"

static char world[MAP_HEIGHT][MAP_WIDTH] = {
    { '#','#','#','#','#','#','#','#','#','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','.','.','#','#','#','#','.','.','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','#','#','#','#','#','#','#','#','#' }
};

void map_print(void)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            printf("%c", world[row][col]);
        }
        printf("\n");
    }
}
