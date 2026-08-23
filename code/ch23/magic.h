#ifndef MAGIC_H
#define MAGIC_H

#include "entity.h"
#include <stddef.h>
#include "rng.h"

/* Status effects, one bit each, so a combatant's whole condition fits in
   a single unsigned and several can be true at once. */
#define MAGIC_SPELLS_PATH "assets/spells.txt"
#define MAGIC_MAX_SPELLS 32
#define MAGIC_NAME_LEN 24

#define STATUS_NONE       0u
#define STATUS_STONESKIN  (1u << 0)   /* self: DEF +40%      */
#define STATUS_HASTENED   (1u << 1)   /* self: AGI +50%      */
#define STATUS_WARDED     (1u << 2)   /* self: absorb damage */
#define STATUS_ASLEEP     (1u << 3)   /* enemy: skips turns  */
#define STATUS_DULLED     (1u << 4)   /* enemy: ATK -25%     */

/* Declared before the function-pointer type that mentions it. */
struct Spell;

/* What a spell needs to do its work. Passing one struct keeps every
   effect function the same shape, which is what lets them live in a
   table together. Target pointers are NULL outside battle. */
typedef struct {
    Player *caster;
    int *target_hp;
    unsigned *target_status;
    int *target_turns;
    Rng *rng;
    char message[64];
} SpellContext;

typedef void (*SpellEffect)(const struct Spell *spell, SpellContext *ctx);

typedef struct Spell {
    /* Owned storage rather than a pointer, so a spell read from a file
       does not depend on a buffer that has since been reused. */
    char name[MAGIC_NAME_LEN];
    int level;               /* level at which it is learned */
    int mp_cost;
    int magnitude;           /* HP restored, damage dealt, or turns */
    unsigned status_flag;    /* which status it applies, or STATUS_NONE */
    SpellEffect effect;
} Spell;

/* Replaces the built-in spell list with one read from disk. Effects are
   selected by name from a fixed set compiled into the game -- behaviour
   cannot be written in a data file. Returns 1 on success; on failure
   returns 0, leaves the built-in list in place, and writes a reason. */
int magic_load_spells(const char *path, char *reason, size_t reason_size);

int magic_spell_count(void);
const Spell *magic_spell_at(int index);

/* How many spells a hero of this level has learned. Because the table is
   ordered by level, the known spells are always the first N. */
int magic_known_count(int hero_level);

/* 1 if the spell must be aimed at an enemy. */
int magic_needs_target(const Spell *spell);

/* Spends MP and runs the effect. Returns 0 (with a reason in
   ctx->message) if the spell cannot be cast. */
int magic_cast(const Spell *spell, SpellContext *ctx);

/* Decrements a status timer by one turn, clearing flags that expire. */
void magic_tick_status(unsigned *status, int *turns);

#endif
