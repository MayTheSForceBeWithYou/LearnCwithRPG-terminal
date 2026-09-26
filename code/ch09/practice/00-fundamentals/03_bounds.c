/*
 * Drill 03 — bounds & configuration edge cases
 *
 * Answer (in code) what the legal camera range is for several MAP/VIEW
 * pairs. Also detect the nonsensical case VIEW > MAP.
 */
#include <stdio.h>

static int g_failures = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_failures++;
    }
}

/* TODO: return the maximum legal camera x (inclusive), or -1 if VIEW_W > MAP_W */
static int max_cam_x(int map_w, int view_w)
{
    (void)map_w;
    (void)view_w;
    return 0;
}

static int max_cam_y(int map_h, int view_h)
{
    (void)map_h;
    (void)view_h;
    return 0;
}

int main(void)
{
    check(max_cam_x(40, 20) == 20, "40/20 -> max 20");
    check(max_cam_y(20, 10) == 10, "20/10 -> max 10");
    check(max_cam_x(40, 40) == 0, "view == map -> locked at 0");
    check(max_cam_x(40, 30) == 10, "40/30 -> max 10");
    check(max_cam_x(40, 41) == -1, "view wider than map is invalid");
    check(max_cam_y(10, 15) == -1, "view taller than map is invalid");

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    puts("03_bounds: all checks passed");
    return 0;
}
