/*
 * Drill 03 — Compare camera policies (self-contained).
 * Fill the three cam_* helpers; make && ./clamp_center
 */
#include <stdio.h>

#define VIEW_WIDTH  20
#define VIEW_HEIGHT 10
#define MAP_WIDTH   40
#define MAP_HEIGHT  20

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

static void cam_center_raw(int tx, int ty, int *cx, int *cy);


static void cam_page(int tx, int ty, int *cx, int *cy);


static void cam_center_clamped(int tx, int ty, int *cx, int *cy);


static int viewport_legal(int cx, int cy)
{
    if (cx < 0 || cy < 0) {
        return 0;
    }
    if (cx > MAP_WIDTH - VIEW_WIDTH) {
        return 0;
    }
    if (cy > MAP_HEIGHT - VIEW_HEIGHT) {
        return 0;
    }
    return 1;
}

int main(void)
{
    const int path[][2] = {
        {1, 1},     /* near top-left corner */
        {10, 5},    /* still near corner region */
        {20, 10},   /* mid-ish */
        {30, 10},   /* mid-right */
        {38, 18},   /* near bottom-right corner */
    };
    const int n = (int)(sizeof path / sizeof path[0]);

    printf("MAP %dx%d  VIEW %dx%d\n", MAP_WIDTH, MAP_HEIGHT,
           VIEW_WIDTH, VIEW_HEIGHT);
    printf("max legal cam = (%d,%d)\n\n",
           MAP_WIDTH - VIEW_WIDTH, MAP_HEIGHT - VIEW_HEIGHT);

    printf("%-10s | %-14s | %-14s | %-14s\n",
           "player", "raw center", "page-snap", "center+clamp");
    printf("%-10s | %-14s | %-14s | %-14s\n",
           "", "cam / ok? / scr", "cam / ok? / scr", "cam / ok? / scr");

    for (int i = 0; i < n; i++) {
        int px = path[i][0];
        int py = path[i][1];
        int r_x, r_y, p_x, p_y, c_x, c_y;

        cam_center_raw(px, py, &r_x, &r_y);
        cam_page(px, py, &p_x, &p_y);
        cam_center_clamped(px, py, &c_x, &c_y);

        printf("(%2d,%2d)   | (%3d,%3d) %s scr(%2d,%2d) | "
               "(%3d,%3d) %s scr(%2d,%2d) | "
               "(%3d,%3d) %s scr(%2d,%2d)\n",
               px, py,
               r_x, r_y, viewport_legal(r_x, r_y) ? "ok " : "BAD",
               px - r_x, py - r_y,
               p_x, p_y, viewport_legal(p_x, p_y) ? "ok " : "BAD",
               px - p_x, py - p_y,
               c_x, c_y, viewport_legal(c_x, c_y) ? "ok " : "BAD",
               px - c_x, py - c_y);
    }

    printf("\nVerdict checks (after you fill TODOs):\n");
    printf("  - At (1,1): raw center should be BAD (negative cam).\n");
    printf("  - At (1,1): center+clamp should be cam(0,0), screen near "
           "(1,1) — @ slides to the corner.\n");
    printf("  - At (20,10): center+clamp screen should be roughly "
           "(%d,%d) — true center.\n",
           VIEW_WIDTH / 2, VIEW_HEIGHT / 2);
    printf("  - Page-snap is also legal on this even-sized map, but screen\n");
    printf("    position jumps (see (20,10): page puts @ at screen (0,0)).\n");
    printf("  - Raw center fails at corners; center+clamp stays legal AND\n");
    printf("    keeps @ centered until an edge forces the slide.\n");
    printf("  - On a non-multiple map (Drill 01 odd case), page-snap needs\n");
    printf("    clamp too — production keeps clamp for both reasons.\n");

    /*
     * TODO 4 (written answer — leave as comments):
     * 1. At player (1,1), raw center cam x = ____ because ____.
     * 2. At (38,18), clamped cam = (____,____); screen pos = (____,____).
     * 3. Deleting clamp after a page-snap rewrite is wrong because ____.
     */
    return 0;
}
