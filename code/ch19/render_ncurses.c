#include <ncurses.h>
#include "render.h"

bool render_init(void)
{
    if (initscr() == NULL) {
        return false;
    }

    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    return true;
}

void render_shutdown(void)
{
    endwin();
}

void render_clear(void)
{
    clear();
}

void render_draw_tile(int x, int y, TileType t)
{
    char symbol = '?';

    switch (t) {
        case TILE_FLOOR:  symbol = '.'; break;
        case TILE_WALL:   symbol = '#'; break;
        case TILE_PLAYER: symbol = '@'; break;
        case TILE_NPC:    symbol = 'o'; break;
        case TILE_DOOR:   symbol = '+'; break;
    }

    mvaddch(y, x, (chtype)symbol);
}

void render_draw_text(int x, int y, const char *s)
{
    mvprintw(y, x, "%s", s);
}

void render_present(void)
{
    refresh();
}
