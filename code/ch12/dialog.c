#include <string.h>
#include "dialog.h"

/* Fills one page starting at dialog->pos, advancing pos past what it
   consumed. Returns the number of lines produced. */
static int fill_page(Dialog *dialog)
{
    const char *text = dialog->source;
    size_t len = strlen(text);
    int width = dialog->width;

    dialog->line_count = 0;

    while (dialog->pos < len && dialog->line_count < DIALOG_MAX_LINES) {
        /* Skip any spaces that would otherwise start a line. */
        while (text[dialog->pos] == ' ') {
            dialog->pos++;
        }
        if (dialog->pos >= len) {
            break;
        }

        /* How much of the remaining text could fit on one line? */
        size_t remaining = len - dialog->pos;
        size_t take = remaining < (size_t)width ? remaining : (size_t)width;

        /* If we are cutting mid-word, back up to the last space. */
        if (take < remaining && text[dialog->pos + take] != ' ') {
            size_t back = take;
            while (back > 0 && text[dialog->pos + back - 1] != ' ') {
                back--;
            }
            /* back == 0 means a single word longer than the line, which
               we have to split rather than loop forever. */
            if (back > 0) {
                take = back;
            }
        }

        /* Drop the trailing space, if the break landed on one. */
        size_t copy = take;
        while (copy > 0 && text[dialog->pos + copy - 1] == ' ') {
            copy--;
        }

        memcpy(dialog->lines[dialog->line_count], text + dialog->pos, copy);
        dialog->lines[dialog->line_count][copy] = '\0';
        dialog->line_count++;

        dialog->pos += take;
    }

    /* Anything left but trailing spaces means there is another page. */
    size_t look = dialog->pos;
    while (look < len && text[look] == ' ') {
        look++;
    }
    dialog->more = (look < len);

    return dialog->line_count;
}

void dialog_start(Dialog *dialog, const char *text, int width)
{
    dialog->line_count = 0;
    dialog->pos = 0;
    dialog->more = 0;
    dialog->source = text;

    if (text == NULL) {
        dialog->width = 1;
        return;
    }

    /* Never allow a caller to ask for more than a line can hold. */
    if (width > DIALOG_LINE_LEN - 1) {
        width = DIALOG_LINE_LEN - 1;
    }
    if (width < 1) {
        width = 1;
    }
    dialog->width = width;

    fill_page(dialog);
}

int dialog_advance(Dialog *dialog)
{
    if (dialog->source == NULL || !dialog->more) {
        dialog->line_count = 0;
        dialog->more = 0;
        return 0;
    }

    fill_page(dialog);
    return 1;
}

void dialog_wrap(Dialog *dialog, const char *text, int width)
{
    dialog_start(dialog, text, width);
}
