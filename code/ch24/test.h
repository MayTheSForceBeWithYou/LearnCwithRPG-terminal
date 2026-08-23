#ifndef TEST_H
#define TEST_H

#include <stdio.h>

/* A whole test framework, in about twenty lines. It does exactly three
   things: count checks, report the ones that failed with a file and
   line, and set the exit status so `make test` fails the build. */

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        tests_run++;                                                    \
        if (!(expr)) {                                                  \
            tests_failed++;                                             \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);    \
        }                                                               \
    } while (0)

#define CHECK_EQ(actual, expected)                                      \
    do {                                                                \
        tests_run++;                                                    \
        int a_ = (actual);                                              \
        int e_ = (expected);                                            \
        if (a_ != e_) {                                                 \
            tests_failed++;                                             \
            printf("  FAIL %s:%d: %s == %d, expected %d\n",             \
                   __FILE__, __LINE__, #actual, a_, e_);                \
        }                                                               \
    } while (0)

static int test_report(void)
{
    printf("\n%d checks, %d failed\n", tests_run, tests_failed);
    return tests_failed != 0;
}

#endif
