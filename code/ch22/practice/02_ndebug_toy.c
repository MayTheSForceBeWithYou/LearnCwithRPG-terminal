#include <stdio.h>
/* Build twice mentally: with and without -DNDEBUG. This binary keeps asserts. */
#include <assert.h>
int main(void)
{
    int x = 1;
    assert(x == 1);
    puts("02_ndebug_toy: all checks passed");
    return 0;
}
