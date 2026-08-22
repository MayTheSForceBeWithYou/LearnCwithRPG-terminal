#ifndef MAP_H
#define MAP_H

#define MAP_HEIGHT 5
#define MAP_WIDTH  10

void map_print(int player_x, int player_y);
int map_is_walkable(int x, int y);

#endif
