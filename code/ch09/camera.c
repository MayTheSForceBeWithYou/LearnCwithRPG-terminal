#include "camera.h"
#include "map.h"

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

void camera_center_on(Camera *cam, int target_x, int target_y)
{
    cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, MAP_WIDTH - VIEW_WIDTH);
    cam->y = clamp(target_y - VIEW_HEIGHT / 2, 0, MAP_HEIGHT - VIEW_HEIGHT);
}

int camera_world_to_screen_x(const Camera *cam, int world_x)
{
    return world_x - cam->x;
}

int camera_world_to_screen_y(const Camera *cam, int world_y)
{
    return world_y - cam->y;
}
