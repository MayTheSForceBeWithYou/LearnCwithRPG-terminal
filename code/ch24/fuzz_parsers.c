/* fuzz_parsers.c -- Chapter 22.

   Every file the game reads is a file a player can edit, which makes
   each parser an untrusted-input boundary. This throws thousands of
   randomly generated files at all five of them and checks two things:

     1. Nothing crashes, over-reads, or leaks.
     2. The good tables are still intact afterwards -- a rejected file
        must leave the previous contents untouched. That is the "no
        partial load" rule from Chapters 20 and 21, tested rather than
        assumed.

   Build and run with:  make fuzz
   Under the sanitizers: make fuzz-sanitize
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rng.h"
#include "map.h"
#include "config.h"
#include "save.h"
#include "party.h"
#include "magic.h"
#include "inventory.h"
#include "paths.h"

#define FUZZ_PATH  "fuzz_tmp.txt"
#define FUZZ_FILES 4000

/* Purely random bytes would almost never produce something a parser
   gets deep into -- it would reject line 1 and stop. Biasing toward the
   characters the parsers actually care about ('=', digits, spaces, '#')
   means far more inputs reach the interesting code paths. */
static char fuzz_char(Rng *rng)
{
    int pick = rng_range(rng, 0, 9);

    if (pick < 3)       return (char)rng_range(rng, '0', '9');
    else if (pick < 5)  return (char)rng_range(rng, 'a', 'z');
    else if (pick == 5) return '=';
    else if (pick == 6) return ' ';
    else if (pick == 7) return '#';
    else if (pick == 8) return '-';
    else                return (char)rng_range(rng, 32, 126);
}

/* Writes one random file. Returns 0 if it could not be created. */
static int write_random_file(Rng *rng, const char *path)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return 0;
    }

    int lines = rng_range(rng, 0, 12);
    for (int i = 0; i < lines; i++) {
        int len = rng_range(rng, 0, 40);
        for (int j = 0; j < len; j++) {
            fputc(fuzz_char(rng), f);
        }
        fputc('\n', f);
    }

    fclose(f);
    return 1;
}

/* Pushes one file through every parser. Each is expected to either
   succeed or fail cleanly -- never to crash, and never to be believed
   without checking its return value. */
static void run_every_parser(const char *path)
{
    char reason[128];

    Map *map = map_load(path);
    if (map != NULL) {
        map_destroy(map);
    }

    Config config;
    config_load(&config, path);

    SaveData save;
    Inventory inv;
    inventory_init(&inv);
    save_read(path, &save, &inv, reason, sizeof reason);
    inventory_free(&inv);

    party_load_levels(path, reason, sizeof reason);
    magic_load_spells(path, reason, sizeof reason);
}

int main(void)
{
    Rng rng;
    rng_seed(&rng, 20260822u);

    /* Load the real tables first, so we can prove they survive. */
    char reason[128];
    char real[512];
    paths_data(real, sizeof real, PARTY_LEVELS_FILE);
    party_load_levels(real, reason, sizeof reason);
    paths_data(real, sizeof real, MAGIC_SPELLS_FILE);
    magic_load_spells(real, reason, sizeof reason);

    const LevelRow *before = party_level_row(1);
    int hp_before = (before != NULL) ? before->hp : -1;
    int spells_before = magic_spell_count();

    for (int i = 0; i < FUZZ_FILES; i++) {
        if (!write_random_file(&rng, FUZZ_PATH)) {
            fprintf(stderr, "could not write %s\n", FUZZ_PATH);
            return 1;
        }
        run_every_parser(FUZZ_PATH);
    }

    remove(FUZZ_PATH);

    printf("fuzzed %d random files through 5 parsers: no crash\n", FUZZ_FILES);

    const LevelRow *after = party_level_row(1);
    int hp_after = (after != NULL) ? after->hp : -1;
    int spells_after = magic_spell_count();

    printf("level 1 hp still %d (expect %d)\n", hp_after, hp_before);
    printf("spell count still %d (expect %d)\n", spells_after, spells_before);

    if (hp_after != hp_before || spells_after != spells_before) {
        printf("FAILED: a rejected file changed a good table\n");
        return 1;
    }

    return 0;
}
