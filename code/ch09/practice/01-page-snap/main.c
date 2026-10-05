/*
 * Drill 01 — Page-snap camera math (self-contained).
 * No game link. Fill the TODOs; make && ./page_snap
 */
#include <stdio.h>

#define VIEW_WIDTH  20
#define VIEW_HEIGHT 10

/* First map: multiples of the view — classic Zelda-style rooms. */
#define MAP_W_EVEN  40
#define MAP_H_EVEN  20

/* Second map: awkward sizes so the last page would overhang. */
#define MAP_W_ODD   45
#define MAP_H_ODD   23

static int clamp(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

/* Snap world coord down to the start of its page. */
static int page_origin(int coord, int view_size);


/* Page camera with no clamp — fine when map dims are multiples of view. */
static void page_camera(int target_x, int target_y, int *cam_x, int *cam_y)
{
    /* Once page_origin is correct, this is the whole function. */
    *cam_x = page_origin(target_x, VIEW_WIDTH);
    *cam_y = page_origin(target_y, VIEW_HEIGHT);
}

static void page_camera_clamped(int target_x, int target_y, int map_w, int map_h, int *cam_x, int *cam_y);


static void demo_even(void)
{
    printf("=== Even map %dx%d (multiples of view) — no clamp needed ===\n",
           MAP_W_EVEN, MAP_H_EVEN);
    printf("%-12s %-12s %-12s %-12s\n",
           "player", "cam (yours)", "cam (expect)", "offset in page");

    const int samples[][2] = {
        {0, 0}, {9, 3}, {19, 9}, {20, 0}, {27, 14}, {39, 19}
    };
    const int n = (int)(sizeof samples / sizeof samples[0]);

    for (int i = 0; i < n; i++) {
        int px = samples[i][0];
        int py = samples[i][1];
        int cx, cy;
        page_camera(px, py, &cx, &cy);

        int expect_x = (px / VIEW_WIDTH) * VIEW_WIDTH;
        int expect_y = (py / VIEW_HEIGHT) * VIEW_HEIGHT;

        printf("(%2d,%2d)      (%2d,%2d)      (%2d,%2d)      (%2d,%2d)\n",
               px, py, cx, cy, expect_x, expect_y,
               px - expect_x, py - expect_y);
    }
    printf("\n");
}

static void demo_odd(void)
{
    printf("=== Odd map %dx%d — page without clamp OVERHANGS ===\n",
           MAP_W_ODD, MAP_H_ODD);
    printf("%-12s %-14s %-14s %-14s\n",
           "player", "page only", "page+clamp", "expect+clamp");

    /* Standing on the last tiles: raw page origin is 40, but max camera
       x is 45 - 20 = 25. Without clamp you'd ask for columns 40..59. */
    const int samples[][2] = {
        {0, 0}, {22, 11}, {40, 20}, {44, 22}
    };
    const int n = (int)(sizeof samples / sizeof samples[0]);

    for (int i = 0; i < n; i++) {
        int px = samples[i][0];
        int py = samples[i][1];

        int raw_x = (px / VIEW_WIDTH) * VIEW_WIDTH;
        int raw_y = (py / VIEW_HEIGHT) * VIEW_HEIGHT;
        int exp_x = clamp(raw_x, 0, MAP_W_ODD - VIEW_WIDTH);
        int exp_y = clamp(raw_y, 0, MAP_H_ODD - VIEW_HEIGHT);

        int cx, cy;
        page_camera_clamped(px, py, MAP_W_ODD, MAP_H_ODD, &cx, &cy);

        printf("(%2d,%2d)      (%2d,%2d)        (%2d,%2d)        (%2d,%2d)\n",
               px, py, raw_x, raw_y, cx, cy, exp_x, exp_y);
    }
    printf("\n");
    printf("Note: at player (44,22), raw page is (40,20) but clamped cam "
           "is (%d,%d).\n",
           MAP_W_ODD - VIEW_WIDTH, MAP_H_ODD - VIEW_HEIGHT);
    printf("That's why the game's lasting camera keeps clamp around — "
           "maps won't always divide evenly.\n");
}

int main(void)
{
    demo_even();
    demo_odd();
    return 0;
}
