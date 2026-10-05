/*
 * Drill 04 — const contract
 *
 * The bad write through a const int * is left active on purpose so this
 * file does not compile until you confront it. Read the error, then comment
 * out the illegal assignment (keep the printf). Re-build; it should pass.
 */
#include <stdio.h>

static void touch_ok(const int *n)
{
    printf("value=%d\n", *n);
    *n = 1; /* TODO: comment this out after you read the compiler error */
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
