#include <stdio.h>
typedef void (*Effect)(int *hp, int mag);
static void heal(int *hp, int mag){ *hp += mag; }
static void harm(int *hp, int mag){ *hp -= mag; }
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    Effect table[] = { heal, harm };
    int hp = 10;
    table[0](&hp, 5);
    check(hp == 15, "heal");
    table[1](&hp, 3);
    check(hp == 12, "harm");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_effect_table: all checks passed");
    return 0;
}
