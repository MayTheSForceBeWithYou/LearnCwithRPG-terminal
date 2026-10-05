#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: base price with optional 10% off (integer); display AND charge use this */
static int entry_price(int base, int has_signet);

int main(void)
{
    check(entry_price(100, 0)==100, "full");
    check(entry_price(100, 1)==90, "10pct");
    check(entry_price(120, 1)==108, "non-round");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_discount_price: all checks passed");
    return 0;
}
