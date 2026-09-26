#include <stdio.h>
typedef enum {
    INPUT_NONE = 0, INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_QUIT
} InputEvent;
enum { KEY_UP = 1000, KEY_DOWN = 1001, KEY_LEFT = 1002, KEY_RIGHT = 1003 };
static InputEvent input_from_key(int key)
{
    switch (key) {
        case 'w': case 'W': case KEY_UP:    return INPUT_UP;
        case 's': case 'S': case KEY_DOWN:  return INPUT_DOWN;
        case 'a': case 'A': case KEY_LEFT:  return INPUT_LEFT;
        case 'd': case 'D': case KEY_RIGHT: return INPUT_RIGHT;
        case 'q': case 'Q':                 return INPUT_QUIT;
        default:                            return INPUT_NONE;
    }
}
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(input_from_key('w') == INPUT_UP, "w");
    check(input_from_key('W') == INPUT_UP, "W");
    check(input_from_key(KEY_UP) == INPUT_UP, "arrow up");
    check(input_from_key('q') == INPUT_QUIT, "q");
    check(input_from_key('z') == INPUT_NONE, "unknown");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03-key-dispatch: all checks passed");
    return 0;
}
