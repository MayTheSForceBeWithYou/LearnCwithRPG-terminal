#include <string.h>
#include "dialog.h"

void dialog_wrap(Dialog *dialog, const char *text, int width)
{
    dialog->line_count = 0;

    if (text == NULL) {
        return;
    }

    /* Never allow a caller to ask for more than a line can hold. */
    if (width > DIALOG_LINE_LEN - 1) {
        width = DIALOG_LINE_LEN - 1;
    }
    if (width < 1) {
        width = 1;
    }

    size_t pos = 0;
    size_t len = strlen(text);

    while (pos < len && dialog->line_count < DIALOG_MAX_LINES) {
        /* Skip any spaces that would otherwise start a line. */
        while (text[pos] == ' ') {
            pos++;
        }
        if (pos >= len) {
            break;
        }

        /* How much of the remaining text could fit on one line? */
        size_t remaining = len - pos;
        size_t take = remaining < (size_t)width ? remaining : (size_t)width;

        /* If we are cutting mid-word, back up to the last space. */
        if (take < remaining && text[pos + take] != ' ') {
            size_t back = take;
            while (back > 0 && text[pos + back - 1] != ' ') {
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
        while (copy > 0 && text[pos + copy - 1] == ' ') {
            copy--;
        }

        memcpy(dialog->lines[dialog->line_count], text + pos, copy);
        dialog->lines[dialog->line_count][copy] = '\0';
        dialog->line_count++;

        pos += take;
    }
}
