/*
 * Drill 01 — row-major offset
 * Calculate the serial memory offset given the row (y), the column (x) and the width of a row (width)
 */
#include <stdio.h>
enum { W = 10, H = 5 };
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
/* TODO */
static int offset(int x, int y, int width);

int main(void){
    check(offset(0,0,W)==0, "origin");
    check(offset(9,0,W)==9, "end of first row");
    check(offset(0,1,W)==10, "start of second row");
    check(offset(3,2,W)==23, "mid");
    check(offset(9,4,W)==49, "last cell of 10x5");
    if(fails){ fprintf(stderr,"%d failed\n",fails); return 1; }
    puts("01_row_major: all checks passed"); return 0;
}
