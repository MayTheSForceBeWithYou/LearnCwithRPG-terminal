/* Drill - loader invariants. Do NOT remove checks from code/ch21/magic.c. */
#include <stdio.h>
#include <string.h>
#define MAX_SPELLS 16
#define NAME_LEN 32
#define EFFECT_LEN 16
typedef struct { int level; char effect[EFFECT_LEN]; char name[NAME_LEN]; } SpellRow;
static const char *KNOWN_EFFECTS[] = { "heal", "damage", "self_status", "enemy_status", "cure", 0 };
static int fails;
static void check(int cond, const char *msg)
{
    if (!cond) { fprintf(stderr, "FAIL: %s" "\n", msg); fails++; }
}
static int is_sorted_by_level(const SpellRow *rows, int n);

static int effect_known(const char *effect);

static int validate_table(const SpellRow *rows, int n, char *reason, size_t reason_size);

static int known_count_prefix(const SpellRow *rows, int n, int hero_level);

static int known_count_scan(const SpellRow *rows, int n, int hero_level)
{
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (rows[i].level <= hero_level) count++;
    }
    return count;
}
static int load_fixture(const char *path, SpellRow *rows, int cap, int *out_n)
{
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "cannot open %s" "\n", path); return 0; }
    char line[128];
    int n = 0;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == "#"[0] || line[0] == 10) continue;
        if (n >= cap) { fclose(f); return 0; }
        if (sscanf(line, "%d %15s %31s", &rows[n].level, rows[n].effect, rows[n].name) != 3) { fclose(f); return 0; }
        n++;
    }
    fclose(f);
    *out_n = n;
    return 1;
}
int main(void)
{
    SpellRow rows[MAX_SPELLS];
    int n = 0;
    char reason[128];
    check(load_fixture("fixtures/ok_sorted.txt", rows, MAX_SPELLS, &n), "load ok_sorted");
    check(validate_table(rows, n, reason, sizeof reason), "ok_sorted should validate");
    check(known_count_prefix(rows, n, 2) == 2, "prefix known at level 2");
    check(known_count_prefix(rows, n, 2) == known_count_scan(rows, n, 2), "prefix matches scan when sorted");
    check(load_fixture("fixtures/unsorted.txt", rows, MAX_SPELLS, &n), "load unsorted");
    check(!validate_table(rows, n, reason, sizeof reason), "unsorted must fail validate");
    printf("unsorted reason: %s" "\n", reason);
    printf("NOTE: prefix=%d scan=%d first=%s" "\n", known_count_prefix(rows, n, 2), known_count_scan(rows, n, 2), rows[0].name);
    check(load_fixture("fixtures/unknown_effect.txt", rows, MAX_SPELLS, &n), "load unknown");
    check(!validate_table(rows, n, reason, sizeof reason), "unknown effect must fail");
    printf("unknown reason: %s" "\n", reason);
    if (fails) { fprintf(stderr, "%d failed" "\n", fails); return 1; }
    puts("loader_invariants: all checks passed");
    return 0;
}
