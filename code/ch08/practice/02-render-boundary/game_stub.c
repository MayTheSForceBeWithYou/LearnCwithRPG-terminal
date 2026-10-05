/*
 * Pretend draw_world. Must call render_present — not a fake refresh.
 */
#include <stdio.h>
#include "render_stub.h"

/* Forbidden in this drill: calling a backend refresh from game code. */
void fake_refresh(void); /* declared only so you can see the name — do not call */

static void draw_world(void);


int main(void)
{
    draw_world();
    draw_world();
    if (render_present_count() != 2) {
        fprintf(stderr, "FAIL: expected 2 present calls, got %d\n",
                render_present_count());
        fprintf(stderr, "hint: call render_present() at end of draw_world\n");
        return 1;
    }
    puts("02-render-boundary: all checks passed");
    return 0;
}
