#include <ncurses.h>
#include "input.h"

InputEvent input_poll(void)
{
    int key = getch();

    switch (key) {
        case 'w': case 'W': return INPUT_UP;
        case 's': case 'S': return INPUT_DOWN;
        case 'a': case 'A': return INPUT_LEFT;
        case 'd': case 'D': return INPUT_RIGHT;
        case 'm': case 'M': return INPUT_MENU;
        case '\n': case '\r': case ' ': return INPUT_CONFIRM;
        case 'f': case 'F': return INPUT_FLEE;
        case 'q': case 'Q': return INPUT_QUIT;
        default: return INPUT_NONE;
    }
}
