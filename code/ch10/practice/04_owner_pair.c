/*
 * Drill 04 — create/destroy ownership pair (tiny "Map"-shaped toy)
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int width;
    int height;
    char *tiles; /* width * height */
} ToyMap;

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO: allocate ToyMap + tiles filled with '.'; border not required */
static ToyMap *toymap_create(int w, int h);


static void toymap_destroy(ToyMap *m);


int main(void)
{
    ToyMap *m = toymap_create(4, 3);
    check(m && m->tiles, "create");
    if (m) {
        check(m->width == 4 && m->height == 3, "size");
        check(m->tiles[0] == '.' && m->tiles[11] == '.', "filled");
        toymap_destroy(m);
    }
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_owner_pair: all checks passed");
    return 0;
}
