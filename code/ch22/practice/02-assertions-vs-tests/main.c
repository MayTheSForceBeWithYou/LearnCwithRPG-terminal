/* Drill - assertions vs tests. Do NOT delete floor in production combat_math. */
#include <stdio.h>
#include <assert.h>

static int fails;
static void check(int cond, const char *msg)
{
    if (!cond) { fprintf(stderr, "FAIL: %s" "\n", msg); fails++; }
}

static int base_damage(int atk, int defense, int level);


int main(void)
{
    /* Table-driven checks — name inputs like a real harness. */
    check(base_damage(5, 0, 40) >= 1, "high level still >= 1");
    check(base_damage(5, 100, 1) == 1, "heavy defense floors to 1");
    check(base_damage(10, 2, 4) >= 1, "ordinary case >= 1");

    puts("With floor: tests describe inputs; assert (when added) names the line.");
    puts("Build with make demo-assert to drop the floor WITHOUT editing production.");

    if (fails) { fprintf(stderr, "%d failed" "\n", fails); return 1; }
    puts("assertions_vs_tests: checks passed");
    return 0;
}
