#include <stdio.h>
#include <string.h>
enum { PHASE_COUNT = 3 };
static const char *lines[PHASE_COUNT] = { "one", "two", "three" };
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: clamp phase into [0, PHASE_COUNT) */
static const char *phase_line(int phase);

int main(void)
{
    check(strcmp(phase_line(0), "one")==0, "0");
    check(strcmp(phase_line(2), "three")==0, "2");
    check(strcmp(phase_line(99), "three")==0, "clamp high");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_phase_index: all checks passed");
    return 0;
}
