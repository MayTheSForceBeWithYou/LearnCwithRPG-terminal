#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "save.h"
#include "party.h"
#include "world.h"

#define SAVE_MAX_LINE 256

/* ---- writing ---- */

int save_write(const char *path, const SaveData *save, const Inventory *inv,
               char *reason, size_t reason_size)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        snprintf(reason, reason_size, "Cannot write %s.", path);
        return 0;
    }

    fprintf(f, "# Some Assembly Required -- save file\n");
    fprintf(f, "# Edit at your own risk. A field out of range is refused.\n");
    fprintf(f, "version = %d\n", SAVE_VERSION);
    fprintf(f, "name = %s\n", save->name);
    fprintf(f, "area = %d\n", save->area);
    fprintf(f, "x = %d\n", save->x);
    fprintf(f, "y = %d\n", save->y);
    fprintf(f, "level = %d\n", save->level);
    fprintf(f, "xp = %d\n", save->xp);
    fprintf(f, "gold = %d\n", save->gold);
    fprintf(f, "hp = %d\n", save->hp);
    fprintf(f, "mp = %d\n", save->mp);
    fprintf(f, "weapon = %d\n", save->equipped[SLOT_WEAPON]);
    fprintf(f, "armour = %d\n", save->equipped[SLOT_ARMOUR]);
    fprintf(f, "accessory = %d\n", save->equipped[SLOT_ACCESSORY]);
    fprintf(f, "rng = %u\n", save->rng_state);

    for (int i = 0; i < inv->count; i++) {
        fprintf(f, "item = %d %d\n",
                (int)inv->stacks[i].id, inv->stacks[i].quantity);
    }

    /* fclose can fail -- the bytes may still be in a buffer that never
       reached the disk. Reporting success without checking would mean
       telling the player their game is saved when it is not. */
    if (fclose(f) != 0) {
        snprintf(reason, reason_size, "Failed to finish writing %s.", path);
        return 0;
    }

    snprintf(reason, reason_size, "Saved.");
    return 1;
}

/* ---- reading ---- */

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                       s[len - 1] == ' '  || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }

    return s;
}

/* Strict integer parse: the whole value must convert, and it must land
   inside [low, high]. */
static int parse_int(const char *value, int low, int high, int *out)
{
    char *end = NULL;
    long parsed = strtol(value, &end, 10);

    if (end == value || *end != '\0') {
        return 0;
    }
    if (parsed < low || parsed > high) {
        return 0;
    }

    *out = (int)parsed;
    return 1;
}

int save_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    }

    fclose(f);
    return 1;
}

int save_read(const char *path, SaveData *save, Inventory *inv,
              char *reason, size_t reason_size)
{
    if (reason_size > 0) {
        reason[0] = '\0';
    }

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        snprintf(reason, reason_size, "No save file.");
        return 0;
    }

    /* Build into locals. Nothing touches the caller's data until every
       field has been read and checked -- validate, then commit, the same
       rule as Chapter 19's shop. */
    SaveData tmp;
    memset(&tmp, 0, sizeof tmp);
    tmp.version = -1;
    tmp.level = -1;

    Inventory tmp_inv;
    inventory_init(&tmp_inv);

    char line[SAVE_MAX_LINE];
    int line_number = 0;
    int failed = 0;

    while (!failed && fgets(line, sizeof line, f) != NULL) {
        line_number++;

        char *text = trim(line);
        if (text[0] == '\0' || text[0] == '#') {
            continue;
        }

        char *equals = strchr(text, '=');
        if (equals == NULL) {
            snprintf(reason, reason_size, "Line %d: expected key = value.",
                     line_number);
            failed = 1;
            break;
        }

        *equals = '\0';
        char *key = trim(text);
        char *value = trim(equals + 1);

        if (strcmp(key, "version") == 0) {
            if (!parse_int(value, 0, 9999, &tmp.version)) {
                snprintf(reason, reason_size, "Line %d: bad version.",
                         line_number);
                failed = 1;
            } else if (tmp.version != SAVE_VERSION) {
                snprintf(reason, reason_size,
                         "Save is version %d, this game reads %d.",
                         tmp.version, SAVE_VERSION);
                failed = 1;
            }
        } else if (strcmp(key, "name") == 0) {
            snprintf(tmp.name, sizeof tmp.name, "%s", value);
        } else if (strcmp(key, "area") == 0) {
            failed = !parse_int(value, 0, AREA_COUNT - 1, &tmp.area);
        } else if (strcmp(key, "x") == 0) {
            failed = !parse_int(value, 0, 9999, &tmp.x);
        } else if (strcmp(key, "y") == 0) {
            failed = !parse_int(value, 0, 9999, &tmp.y);
        } else if (strcmp(key, "level") == 0) {
            failed = !parse_int(value, 1, PARTY_MAX_LEVEL, &tmp.level);
        } else if (strcmp(key, "xp") == 0) {
            failed = !parse_int(value, 0, 1000000, &tmp.xp);
        } else if (strcmp(key, "gold") == 0) {
            failed = !parse_int(value, 0, 1000000, &tmp.gold);
        } else if (strcmp(key, "hp") == 0) {
            failed = !parse_int(value, 0, 100000, &tmp.hp);
        } else if (strcmp(key, "mp") == 0) {
            failed = !parse_int(value, 0, 100000, &tmp.mp);
        } else if (strcmp(key, "weapon") == 0) {
            failed = !parse_int(value, 0, gear_count(SLOT_WEAPON) - 1,
                                &tmp.equipped[SLOT_WEAPON]);
        } else if (strcmp(key, "armour") == 0) {
            failed = !parse_int(value, 0, gear_count(SLOT_ARMOUR) - 1,
                                &tmp.equipped[SLOT_ARMOUR]);
        } else if (strcmp(key, "accessory") == 0) {
            failed = !parse_int(value, 0, gear_count(SLOT_ACCESSORY) - 1,
                                &tmp.equipped[SLOT_ACCESSORY]);
        } else if (strcmp(key, "rng") == 0) {
            int parsed = 0;
            if (parse_int(value, 0, 2147483647, &parsed)) {
                tmp.rng_state = (unsigned)parsed;
            } else {
                failed = 1;
            }
        } else if (strcmp(key, "item") == 0) {
            int id = 0;
            int qty = 0;

            if (sscanf(value, "%d %d", &id, &qty) != 2 ||
                id < 0 || id >= ITEM_TYPE_COUNT || qty <= 0 || qty > 99) {
                snprintf(reason, reason_size, "Line %d: bad item entry.",
                         line_number);
                failed = 1;
            } else if (!inventory_add(&tmp_inv, (ItemId)id, qty)) {
                snprintf(reason, reason_size, "Out of memory reading items.");
                failed = 1;
            }
        } else {
            snprintf(reason, reason_size, "Line %d: unknown key '%s'.",
                     line_number, key);
            failed = 1;
        }

        if (failed && reason[0] == '\0') {
            snprintf(reason, reason_size, "Line %d: bad value for '%s'.",
                     line_number, key);
        }
    }

    fclose(f);

    /* Required fields must actually have appeared. A file missing its
       version line is not a version 0 save, it is not a save. */
    if (!failed && tmp.version != SAVE_VERSION) {
        snprintf(reason, reason_size, "Save has no version line.");
        failed = 1;
    }
    if (!failed && tmp.level < 1) {
        snprintf(reason, reason_size, "Save has no level line.");
        failed = 1;
    }
    if (!failed && tmp.name[0] == '\0') {
        snprintf(reason, reason_size, "Save has no name line.");
        failed = 1;
    }

    if (failed) {
        inventory_free(&tmp_inv);
        return 0;
    }

    /* Everything checked. Now commit. */
    inventory_free(inv);
    *inv = tmp_inv;
    *save = tmp;

    snprintf(reason, reason_size, "Loaded.");
    return 1;
}
