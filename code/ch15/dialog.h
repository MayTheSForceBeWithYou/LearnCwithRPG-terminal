#ifndef DIALOG_H
#define DIALOG_H

#define DIALOG_MAX_LINES 4
#define DIALOG_LINE_LEN  48

typedef struct {
    char lines[DIALOG_MAX_LINES][DIALOG_LINE_LEN];
    int line_count;
} Dialog;

/* Word-wraps text into dialog->lines, breaking at spaces where possible.
   Never writes more than DIALOG_LINE_LEN - 1 characters plus a NUL into
   any line, and never fills more than DIALOG_MAX_LINES lines; text that
   does not fit is dropped. width is clamped to the line capacity. */
void dialog_wrap(Dialog *dialog, const char *text, int width);

#endif
