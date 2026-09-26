#include <stdio.h>
static void touch_ok(const int *n)
{
    printf("value=%d\n", *n);
}
static int identity(const int *n)
{
    return *n;
}
int main(void)
{
    int x = 42;
    touch_ok(&x);
    if (identity(&x) != 42) {
        fprintf(stderr, "FAIL identity\n");
        return 1;
    }
    puts("04_const_contract: all checks passed");
    return 0;
}
