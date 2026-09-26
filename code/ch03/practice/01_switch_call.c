#include <stdio.h>
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO: return 1 if c is h/H/t/T else 0 */
static int valid_call(char c)
{
    (void)c; return 0;
}
int main(void)
{
    check(valid_call('h')==1 && valid_call('T')==1, "ok");
    check(valid_call('x')==0, "bad");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01_switch_call: all checks passed");
    return 0;
}
