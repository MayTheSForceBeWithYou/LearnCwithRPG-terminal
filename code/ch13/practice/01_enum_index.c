#include <stdio.h>
typedef enum { AREA_A, AREA_B, AREA_C, AREA_COUNT } AreaId;
static const char *names[AREA_COUNT] = { "A", "B", "C" };
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    check(AREA_COUNT == 3, "count trick");
    check(names[AREA_B][0] == 'B', "index");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_enum_index: all checks passed");
    return 0;
}
