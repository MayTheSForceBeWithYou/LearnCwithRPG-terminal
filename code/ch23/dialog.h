#ifndef DIALOG_H
#define DIALOG_H

#include <stddef.h>

#define DIALOG_MAX_LINES 4
#define DIALOG_LINE_LEN  48

typedef struct {
    char lines[DIALOG_MAX_LINES][DIALOG_LINE_LEN];
    int line_count;

    /* Where the next page starts. The Dialog does NOT own source -- the
       caller must keep that text alive for as long as the dialog is on
       screen. String literals and entries in a static table qualify. */
    const char *source;
    size_t pos;
    int width;
    int more;          /* 1 if text remains after the current page */
} Dialog;

/* Begins a conversation and produces the first page. Word-wraps at
   spaces where possible; never writes more than DIALOG_LINE_LEN - 1
   characters plus a NUL into a line, and never fills more than
   DIALOG_MAX_LINES lines. width is clamped to the line capacity. */
void dialog_start(Dialog *dialog, const char *text, int width);

/* Produces the next page. Returns 1 if there was more to show, 0 if the
   conversation is over (in which case the dialog is left empty). */
int dialog_advance(Dialog *dialog);

/* Convenience for one-shot text that is known to fit: same as
   dialog_start, kept because most callers do not care about paging. */
void dialog_wrap(Dialog *dialog, const char *text, int width);

#endif
