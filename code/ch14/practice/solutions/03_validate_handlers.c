#include <stdio.h>
typedef void (*Fn)(void);
typedef struct { Fn draw; Fn handle; } Mode;
static void d(void){}
static void h(void){}
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int validate(const Mode *t, int n)
{
    for (int i = 0; i < n; i++)
        if (!t[i].draw || !t[i].handle) return 0;
    return 1;
}
int main(void)
{
    Mode ok[] = { {d,h}, {d,h} };
    Mode bad[] = { {d,h}, {NULL,h} };
    check(validate(ok, 2)==1, "ok");
    check(validate(bad, 2)==0, "bad");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("03_validate_handlers: all checks passed");
    return 0;
}
