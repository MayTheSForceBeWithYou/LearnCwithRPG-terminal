#include <stdio.h>
typedef struct { int x; int y; } Point;
static void fill_point(Point *p, int x, int y)
{
    p->x = x;
    p->y = y;
}
int main(void)
{
    Point pt = {0, 0};
    fill_point(&pt, 9, 4);
    if (pt.x != 9 || pt.y != 4) {
        fprintf(stderr, "FAIL fill_point\n");
        return 1;
    }
    puts("05_dangling_demo: all checks passed");
    return 0;
}
