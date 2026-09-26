/*
 * Drill 02 — always check malloc; cleanup partial failure
 *
 * Implement make_pair: allocate two ints. If the second malloc fails,
 * free the first and return NULL.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *a;
    int *b;
} Pair;

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

/* TODO */
static Pair *make_pair(int av, int bv)
{
    (void)av; (void)bv;
    return NULL;
}

static void free_pair(Pair *p)
{
    if (!p) return;
    free(p->a);
    free(p->b);
    free(p);
}

int main(void)
{
    Pair *p = make_pair(1, 2);
    check(p != NULL && p->a && p->b, "created");
    if (p) {
        check(*p->a == 1 && *p->b == 2, "values");
        free_pair(p);
    }
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("02_null_check: all checks passed");
    return 0;
}
