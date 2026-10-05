/*
 * Drill 03 — grow a buffer with realloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO: grow *buf to at least new_cap ints; update *cap; return 1 ok / 0 fail */
static int grow(int **buf, int *cap, int new_cap);


int main(void)
{
    int cap = 2;
    int *buf = malloc((size_t)cap * sizeof *buf);
    check(buf != NULL, "initial");
    if (!buf) return 1;
    buf[0] = 1; buf[1] = 2;
    check(grow(&buf, &cap, 4) == 1, "grow");
    check(cap >= 4, "cap");
    buf[2] = 3; buf[3] = 4;
    check(buf[0] == 1 && buf[3] == 4, "preserved");
    free(buf);
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_grow_buffer: all checks passed");
    return 0;
}
