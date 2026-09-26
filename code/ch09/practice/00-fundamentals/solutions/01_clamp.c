/*
 * Solution — Drill 01 clamp
 */
#include <stdio.h>

static int g_failures = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_failures++;
    }
}

static int clamp(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

int main(void)
{
    check(clamp(5, 0, 10) == 5, "inside range stays put");
    check(clamp(-3, 0, 10) == 0, "below low -> low");
    check(clamp(99, 0, 10) == 10, "above high -> high");
    check(clamp(0, 0, 10) == 0, "equal to low");
    check(clamp(10, 0, 10) == 10, "equal to high");
    check(clamp(7, 7, 7) == 7, "degenerate single-point range");
    check(clamp(25 - 10, 0, 40 - 20) == 15, "center-ish cam x");
    check(clamp(1 - 10, 0, 40 - 20) == 0, "near left edge clamps to 0");
    check(clamp(39 - 10, 0, 40 - 20) == 20, "near right edge clamps to max");

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    puts("01_clamp: all checks passed");
    return 0;
}
