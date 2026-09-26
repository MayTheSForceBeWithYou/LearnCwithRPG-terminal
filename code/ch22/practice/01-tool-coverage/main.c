/* Drill - tool coverage. Do NOT delete memset in code/ch22 entity_create_player. */
#include <stdio.h>
#include <string.h>

typedef struct { int hp; int mp; char ready; } Hero;

static int fails;
static void check(int cond, const char *msg)
{
    if (!cond) { fprintf(stderr, "FAIL: %s" "\n", msg); fails++; }
}

/* Intentional bug stand-in: ready left uninitialized (no memset). */
static Hero buggy_create(void)
{
    Hero h;
    h.hp = 30;
    h.mp = 10;
    /* TODO1: leave h.ready unset on purpose for the coverage lesson.
       Do not fix this the way production uses memset — the point
       is observing which harness path notices. */
    return h;
}

static int read_ready(Hero h)
{
    return h.ready != 0; /* uses the field — like party clamps on hp */
}

/* Simulates make valgrind / run_tests — never calls buggy_create. */
static int test_suite_path(void)
{
    Hero ok;
    memset(&ok, 0, sizeof ok);
    ok.hp = 1;
    return ok.hp == 1;
}

/* Simulates playing the game — does call buggy_create. */
static int covered_path(void)
{
    Hero h = buggy_create();
    return read_ready(h) || h.hp == 30; /* forces a use of h */
}

int main(void)
{
    check(test_suite_path(), "test_suite_path should pass");
    check(covered_path(), "covered_path runs buggy_create");
    puts("Coverage lesson:");
    puts("  test_suite_path never calls buggy_create -> tools stay quiet");
    puts("  covered_path does call it -> memcheck/ASan can see the bug");
    puts("  Production: keep memset; practice the coverage idea HERE.");
    if (fails) return 1;
    puts("tool_coverage: checks passed");
    return 0;
}
