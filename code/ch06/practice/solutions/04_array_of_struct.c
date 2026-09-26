#include <stdio.h>
typedef struct { int hp; } Member;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    Member party[3] = { {20}, {15}, {10} };
    int sum = 0;
    for (int i = 0; i < 3; i++) sum += party[i].hp;
    check(sum == 45, "party hp");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_array_of_struct: all checks passed");
    return 0;
}
