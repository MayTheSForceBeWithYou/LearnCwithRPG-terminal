#include <stdio.h>
typedef int (*Op)(int,int);
static int add(int a,int b){return a+b;}
static int sub(int a,int b){return a-b;}
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    /* TODO: a two-entry table of Op — add then sub */
    Op table[] = TABLE_INIT;
    check(table[0](10,3)==13, "add idx");
    check(table[1](10,3)==7, "sub idx");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_dispatch_table: all checks passed");
    return 0;
}
