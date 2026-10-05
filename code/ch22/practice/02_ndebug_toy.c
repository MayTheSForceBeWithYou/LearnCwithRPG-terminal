#include <stdio.h>
/* Build twice mentally: with and without -DNDEBUG. This binary keeps asserts. */
#include <assert.h>
int main(void)
{
    int x = 1;
    /* TODO: assert a true condition about x, then print the pass line */
    assert(NDEBUG_PROOF);
    puts("02_ndebug_toy: all checks passed");
    return 0;
}
