/*
 * Drill 02 — Dead-zone camera (self-contained).
 * Fill TODO in camera_update_dead_zone; make && ./dead_zone
 */
#include <stdio.h>

#define VIEW_WIDTH  20
#define VIEW_HEIGHT 10
#define MAP_WIDTH   40
#define MAP_HEIGHT  20

/* Dead-zone margins in screen cells. Player may freely occupy
   [MARGIN_X .. VIEW_WIDTH-1-MARGIN_X] on x (same idea on y). */
#define MARGIN_X  6
#define MARGIN_Y  3

typedef struct {
    int x;
    int y;
} Camera;

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

static void camera_center_on(Camera *cam, int target_x, int target_y)
{
    cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, MAP_WIDTH - VIEW_WIDTH);
    cam->y = clamp(target_y - VIEW_HEIGHT / 2, 0, MAP_HEIGHT - VIEW_HEIGHT);
}

/*
 * Update camera in place. Only move the camera when the player's
 * screen-space position would leave the dead-zone box.
 */
static void camera_update_dead_zone(Camera *cam, int world_x, int world_y);


static void print_step(int step, int px, int py, const Camera *cam)
{
    int sx = px - cam->x;
    int sy = py - cam->y;
    printf("step %2d  world(%2d,%2d)  cam(%2d,%2d)  screen(%2d,%2d)\n",
           step, px, py, cam->x, cam->y, sx, sy);
}

int main(void)
{
    /* Scripted walk: start centered, drift inside the zone, then push
       right hard enough to drag the camera, then push down. */
    int px = 15;
    int py = 8;
    Camera cam;
    camera_center_on(&cam, px, py);

    printf("Margins: x=%d..%d  y=%d..%d (screen space)\n\n",
           MARGIN_X, VIEW_WIDTH - 1 - MARGIN_X,
           MARGIN_Y, VIEW_HEIGHT - 1 - MARGIN_Y);

    print_step(0, px, py, &cam);

    /* +1 x, five times — should stay inside dead zone; cam unchanged */
    for (int i = 1; i <= 5; i++) {
        px += 1;
        camera_update_dead_zone(&cam, px, py);
        print_step(i, px, py, &cam);
    }

    /* Keep walking right — eventually screen_x hits the right margin */
    for (int i = 6; i <= 14; i++) {
        px += 1;
        camera_update_dead_zone(&cam, px, py);
        print_step(i, px, py, &cam);
    }

    /* Walk down past the bottom margin */
    for (int i = 15; i <= 22; i++) {
        py += 1;
        camera_update_dead_zone(&cam, px, py);
        print_step(i, px, py, &cam);
    }

    printf("\nIf your dead zone works:\n");
    printf("  - early steps: cam stays put, screen x drifts upward\n");
    printf("  - later steps: screen x sticks near %d while cam.x grows\n",
           VIEW_WIDTH - 1 - MARGIN_X);
    printf("  - downward walk: screen y sticks near %d while cam.y grows\n",
           VIEW_HEIGHT - 1 - MARGIN_Y);
    return 0;
}
