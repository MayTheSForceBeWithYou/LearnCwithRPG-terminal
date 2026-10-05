#include <stdio.h>
static int add(int a, int b){ return a+b; }
static int mul(int a, int b){ return a*b; }
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    /* TODO: declare a function pointer named op and point it at add, then mul */
    OP_DECL;
    check(op(2,3)==5, "add");
    op = mul;
    check(op(2,3)==6, "mul");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_fnptr_basics: all checks passed");
    return 0;
}
