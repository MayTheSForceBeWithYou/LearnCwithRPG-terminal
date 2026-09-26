/*
 * Solution — Drill 02 center
 */
#include <stdio.h>

enum { MAP_W = 40, MAP_H = 20, VIEW_W = 20, VIEW_H = 10 };

static int g_failures = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_failures++;
    }
}

static int clamp(int value, int low, int high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static void camera_center(int target_x, int target_y, int *cam_x, int *cam_y)
{
    *cam_x = clamp(target_x - VIEW_W / 2, 0, MAP_W - VIEW_W);
    *cam_y = clamp(target_y - VIEW_H / 2, 0, MAP_H - VIEW_H);
}

int main(void)
{
    int cx, cy;

    camera_center(0, 0, &cx, &cy);
    check(cx == 0 && cy == 0, "top-left corner pins camera at 0,0");

    camera_center(MAP_W - 1, MAP_H - 1, &cx, &cy);
    check(cx == MAP_W - VIEW_W && cy == MAP_H - VIEW_H,
          "bottom-right pins camera at max");

    camera_center(20, 10, &cx, &cy);
    check(cx == 20 - VIEW_W / 2 && cy == 10 - VIEW_H / 2,
          "mid-map centers exactly");

    camera_center(10, 5, &cx, &cy);
    check(cx == 0 && cy == 0, "near top-left still clamped");

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    puts("02_center: all checks passed");
    return 0;
}
