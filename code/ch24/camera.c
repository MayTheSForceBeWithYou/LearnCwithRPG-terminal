#include "camera.h"

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

void camera_center_on(Camera *cam, int target_x, int target_y,
                      int map_width, int map_height)
{
    cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, map_width - VIEW_WIDTH);
    cam->y = clamp(target_y - VIEW_HEIGHT / 2, 0, map_height - VIEW_HEIGHT);
}

int camera_world_to_screen_x(const Camera *cam, int world_x)
{
    return world_x - cam->x;
}

int camera_world_to_screen_y(const Camera *cam, int world_y)
{
    return world_y - cam->y;
}
