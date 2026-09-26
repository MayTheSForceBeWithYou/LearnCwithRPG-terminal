#include <stdio.h>
#include <stdlib.h>
typedef struct {
    int width;
    int height;
    char *tiles;
} ToyMap;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static ToyMap *toymap_create(int w, int h)
{
    if (w <= 0 || h <= 0) return NULL;
    ToyMap *m = malloc(sizeof *m);
    if (!m) return NULL;
    m->width = w;
    m->height = h;
    m->tiles = malloc((size_t)w * (size_t)h);
    if (!m->tiles) { free(m); return NULL; }
    for (int i = 0; i < w * h; i++) m->tiles[i] = '.';
    return m;
}
static void toymap_destroy(ToyMap *m)
{
    if (!m) return;
    free(m->tiles);
    free(m);
}
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
