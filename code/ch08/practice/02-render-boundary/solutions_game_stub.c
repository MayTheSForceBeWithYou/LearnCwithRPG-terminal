#include <stdio.h>
#include "render_stub.h"
static void draw_world(void)
{
    render_draw('@');
    render_present();
}
int main(void)
{
    draw_world();
    draw_world();
    if (render_present_count() != 2) {
        fprintf(stderr, "FAIL: expected 2 present calls, got %d\n",
                render_present_count());
        return 1;
    }
    puts("02-render-boundary: all checks passed");
    return 0;
}
