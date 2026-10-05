/*
 * Drill 05 — dangling pointer anti-pattern (READ carefully)
 *
 * DO NOT uncomment return_local_addr and call it. Returning &local is
 * undefined behaviour: the stack slot is gone after return.
 *
 * Instead, implement fill_point to write through an out-param owned by
 * the caller — the durable pattern entity_move / minmax use.
 */
#include <stdio.h>

typedef struct { int x; int y; } Point;

/* TODO: set p->x and p->y */
static void fill_point(Point *p, int x, int y);


/*
 * BROKEN PATTERN — for reading only:
 *
 * static Point *return_local_addr(void)
 * {
 *     Point local = {1, 2};
 *     return &local;  // dangling the instant this returns
 * }
 */

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
