#ifndef MAGIC_H
#define MAGIC_H

#include "entity.h"
#include "rng.h"

/* Status effects, one bit each, so a combatant's whole condition fits in
   a single unsigned and several can be true at once. */
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
    const char *name;
    int level;               /* level at which it is learned */
    int mp_cost;
    int magnitude;           /* HP restored, damage dealt, or turns */
    unsigned status_flag;    /* which status it applies, or STATUS_NONE */
    SpellEffect effect;
} Spell;

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
