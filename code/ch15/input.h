#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_MENU,
    INPUT_CONFIRM,
    INPUT_QUIT
} InputEvent;

InputEvent input_poll(void);

#endif
