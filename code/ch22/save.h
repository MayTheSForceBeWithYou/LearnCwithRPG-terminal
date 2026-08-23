#ifndef SAVE_H
#define SAVE_H

#include <stddef.h>
#include "entity.h"
#include "inventory.h"
#include "rng.h"

/* Bump this whenever the set of fields changes. A save written by an
   older build is refused rather than misread -- see save_load. */
#define SAVE_VERSION 1

#define SAVE_PATH "assets/save.txt"

/* Everything a run consists of. Deliberately a plain snapshot rather
   than pointers into the live game, so writing it cannot be confused
   with mutating it. */
typedef struct {
    int version;
    char name[64];
    int area;
    int x;
    int y;
    int level;
    int xp;
    int gold;
    int hp;
    int mp;
    int equipped[SLOT_COUNT];
    unsigned rng_state;
} SaveData;

/* Writes save plus the inventory. Returns 1 on success; on failure
   returns 0 and writes a reason into reason (reason_size bytes). */
int save_write(const char *path, const SaveData *save, const Inventory *inv,
               char *reason, size_t reason_size);

/* Reads a save. On success fills *save and *inv and returns 1. On ANY
   problem -- missing file, wrong version, bad field, unknown key --
   returns 0 with a specific reason, having left *save and *inv
   untouched. There is no partial load. */
int save_read(const char *path, SaveData *save, Inventory *inv,
              char *reason, size_t reason_size);

int save_exists(const char *path);

#endif
