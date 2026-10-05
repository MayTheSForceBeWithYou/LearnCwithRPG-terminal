/*
 * Drill 05 — visibility / on-screen test
 *
 * A world point is on screen iff:
 *   cam_x <= world_x < cam_x + VIEW_W
 *   cam_y <= world_y < cam_y + VIEW_H
 *
 * This is the helper later chapters need when drawing NPCs.
 */
#include <stdio.h>

enum { VIEW_W = 20, VIEW_H = 10 };

static int g_failures = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_failures++;
    }
}

/* TODO */
static int camera_is_on_screen(int cam_x, int cam_y, int world_x, int world_y);


int main(void)
{
    int cx = 10, cy = 5;

    check(camera_is_on_screen(cx, cy, 10, 5) == 1, "top-left of view inclusive");
    check(camera_is_on_screen(cx, cy, 29, 14) == 1, "bottom-right inclusive (cam+VIEW-1)");
    check(camera_is_on_screen(cx, cy, 30, 5) == 0, "one past right edge exclusive");
    check(camera_is_on_screen(cx, cy, 10, 15) == 0, "one past bottom exclusive");
    check(camera_is_on_screen(cx, cy, 9, 5) == 0, "just left of view");
    check(camera_is_on_screen(cx, cy, 10, 4) == 0, "just above view");

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    puts("05_visibility: all checks passed");
    return 0;
}
