/*
 * READING-ONLY LEAK EXHIBIT — do not copy this pattern into the chapter game.
 *
 * Same "game" as game_stub.c, but draw_world calls fake_refresh() directly,
 * bypassing render_present. Built only via `make leak` with
 * -DEXPOSE_FAKE_REFRESH so the stub exports that symbol.
 *
 * Analog in the real chapter: deleting render_present and calling refresh()
 * from draw_world after #include <ncurses.h> — lasting-path vandalism.
 * Study the dependency cost; leave the playable wrappers intact.
 *
 * Unworked gate: remove the #error after reading this header, then `make leak`.
 */
#error "Read the file header (leak exhibit). Remove this #error, then: make leak"
#include <stdio.h>
#include "render_stub.h"

/* Backend name — not part of render_stub.h. Leak requires a stray declaration. */
void fake_refresh(void);

static void draw_world(void)
{
    render_draw('@');
    /* Wrong lasting habit: game names the backend flush. */
    fake_refresh();
}

int main(void)
{
    draw_world();
    draw_world();
    printf("02-render-boundary LEAK exhibit: fake_refresh called from game logic\n");
    printf("(present-equivalent flushes=%d — behavior can look fine today)\n",
           render_present_count());
    printf("Do NOT copy this into the chapter game. Lasting path keeps render_present.\n");
    return 0;
}
