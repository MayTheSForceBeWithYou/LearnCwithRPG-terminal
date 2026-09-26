/*
 * Drill 04 — world <-> screen conversion
 *
 * screen = world - camera
 * world  = screen + camera
 *
 * Round-trips must hold for any camera and any world point (mathematically;
 * visibility is a separate question — see drill 05).
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

/* TODO */
static int world_to_screen(int camera, int world)
{
    (void)camera;
    (void)world;
    return 0;
}

static int screen_to_world(int camera, int screen)
{
    (void)camera;
    (void)screen;
    return 0;
}

int main(void)
{
    int cam = 12;
    check(world_to_screen(cam, 12) == 0, "world at camera origin -> screen 0");
    check(world_to_screen(cam, 17) == 5, "world 17 -> screen 5");
    check(screen_to_world(cam, 5) == 17, "inverse");
    check(screen_to_world(cam, world_to_screen(cam, 30)) == 30, "round trip A");
    check(world_to_screen(cam, screen_to_world(cam, 3)) == 3, "round trip B");

    /* Negative screen means "left of the viewport" — still a valid conversion */
    check(world_to_screen(cam, 5) == -7, "off-screen left gives negative screen");

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    puts("04_world_to_screen: all checks passed");
    return 0;
}
