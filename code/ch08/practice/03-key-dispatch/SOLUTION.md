# Solution

```c
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
```

Same shape as live `input_poll` — practice the table here, then add
`KEY_*` to the real file as the durable UX step.
