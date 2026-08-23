#ifndef CAMERA_H
#define CAMERA_H

#define VIEW_WIDTH  20
#define VIEW_HEIGHT 10

typedef struct {
    int x;
    int y;
} Camera;

void camera_center_on(Camera *cam, int target_x, int target_y,
                      int map_width, int map_height);
int camera_world_to_screen_x(const Camera *cam, int world_x);
int camera_world_to_screen_y(const Camera *cam, int world_y);

#endif
