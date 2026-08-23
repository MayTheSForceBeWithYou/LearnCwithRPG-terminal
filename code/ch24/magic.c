#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "magic.h"
#include "combat_math.h"
#include "party.h"

/* ---- the effects ----
   Every one has the same signature, which is the whole point: it lets
   them sit in a table and be selected by data instead of by a switch. */

static void effect_heal(const Spell *spell, SpellContext *ctx)
{
    int before = ctx->caster->hp;

    ctx->caster->hp += spell->magnitude;
    if (ctx->caster->hp > ctx->caster->max_hp) {
        ctx->caster->hp = ctx->caster->max_hp;
    }

    snprintf(ctx->message, sizeof ctx->message, "%s restores %d HP.",
             spell->name, ctx->caster->hp - before);
}

static void effect_damage(const Spell *spell, SpellContext *ctx)
{
    /* Spells ignore physical defence entirely (Checkpoint C) but take
       the same variance band as a weapon swing. */
    int roll = rng_range(ctx->rng, 0, 2 * COMBAT_VARIANCE_PERCENT);
    int damage = combat_apply_variance(spell->magnitude, roll);

    *ctx->target_hp -= damage;

    snprintf(ctx->message, sizeof ctx->message, "%s hits for %d.",
             spell->name, damage);
}

static void effect_self_status(const Spell *spell, SpellContext *ctx)
{
    ctx->caster->status |= spell->status_flag;
    ctx->caster->status_turns = spell->magnitude;

    snprintf(ctx->message, sizeof ctx->message, "%s takes hold.",
             spell->name);
}

static void effect_enemy_status(const Spell *spell, SpellContext *ctx)
{
    *ctx->target_status |= spell->status_flag;
    *ctx->target_turns = spell->magnitude;

    snprintf(ctx->message, sizeof ctx->message, "%s lands.", spell->name);
}

static void effect_cure(const Spell *spell, SpellContext *ctx)
{
    ctx->caster->status = STATUS_NONE;
    ctx->caster->status_turns = 0;

    snprintf(ctx->message, sizeof ctx->message, "%s clears your head.",
             spell->name);
}

/* ---- the table ----
   Thirteen spells, five effect functions. Adding a spell is adding a row;
   adding a *kind* of spell is adding a function. That separation is the
   payoff for Chapter 14's dispatch table. */
static const Spell builtin_spells[] = {
    /* name            lv  mp  mag  status flag        effect */
    { "Mend",           2,  3,  35, STATUS_NONE,      effect_heal },
    { "Ember",          3,  4,  25, STATUS_NONE,      effect_damage },
    { "Dull",           5,  4,   4, STATUS_DULLED,    effect_enemy_status },
    { "Frost Nip",      6,  6,  40, STATUS_NONE,      effect_damage },
    { "Adjourn",        7,  7,   3, STATUS_ASLEEP,    effect_enemy_status },
    { "Purge",          8,  5,   0, STATUS_NONE,      effect_cure },
    { "Stoneskin",      9,  8,   5, STATUS_STONESKIN, effect_self_status },
    { "Greater Mend",  10, 10, 110, STATUS_NONE,      effect_heal },
    { "Jolt",          12, 11,  70, STATUS_NONE,      effect_damage },
    { "Hasten",        13, 12,   5, STATUS_HASTENED,  effect_self_status },
    { "Immolate",      15, 18, 130, STATUS_NONE,      effect_damage },
    { "Ward",          16, 14,   5, STATUS_WARDED,    effect_self_status },
    { "Sunder",        18, 30, 240, STATUS_NONE,      effect_damage }
};

#define BUILTIN_COUNT ((int)(sizeof builtin_spells / sizeof builtin_spells[0]))

/* The list actually in use, and how many of it are real. */
static Spell spell_table[MAGIC_MAX_SPELLS];
static int spell_count = 0;

static void ensure_spells(void)
{
    if (spell_count == 0) {
        memcpy(spell_table, builtin_spells, sizeof builtin_spells);
        spell_count = BUILTIN_COUNT;
    }
}

/* The only effects that exist. A data file selects from this list by
   name; it cannot introduce a new one, because behaviour is code. */
typedef struct {
    const char *name;
    SpellEffect fn;
} EffectBinding;

static const EffectBinding effect_bindings[] = {
    { "heal",         effect_heal },
    { "damage",       effect_damage },
    { "self_status",  effect_self_status },
    { "enemy_status", effect_enemy_status },
    { "cure",         effect_cure }
};

typedef struct {
    const char *name;
    unsigned flag;
} StatusBinding;

static const StatusBinding status_bindings[] = {
    { "none",      STATUS_NONE },
    { "stoneskin", STATUS_STONESKIN },
    { "hastened",  STATUS_HASTENED },
    { "warded",    STATUS_WARDED },
    { "asleep",    STATUS_ASLEEP },
    { "dulled",    STATUS_DULLED }
};

static SpellEffect lookup_effect(const char *name)
{
    for (size_t i = 0; i < sizeof effect_bindings / sizeof effect_bindings[0];
         i++) {
        if (strcmp(effect_bindings[i].name, name) == 0) {
            return effect_bindings[i].fn;
        }
    }

    return NULL;
}

static int lookup_status(const char *name, unsigned *out)
{
    for (size_t i = 0; i < sizeof status_bindings / sizeof status_bindings[0];
         i++) {
        if (strcmp(status_bindings[i].name, name) == 0) {
            *out = status_bindings[i].flag;
            return 1;
        }
    }

    return 0;
}

int magic_load_spells(const char *path, char *reason, size_t reason_size)
{
    ensure_spells();

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        snprintf(reason, reason_size, "no %s, using built-in spells", path);
        return 0;
    }

    Spell parsed[MAGIC_MAX_SPELLS];
    int count = 0;

    char line[256];
    int line_number = 0;
    int failed = 0;

    while (!failed && fgets(line, sizeof line, f) != NULL) {
        line_number++;

        char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') {
            continue;
        }

        if (count >= MAGIC_MAX_SPELLS) {
            snprintf(reason, reason_size, "%s: more than %d spells",
                     path, MAGIC_MAX_SPELLS);
            failed = 1;
            break;
        }

        char keyword[16];
        char status_name[16];
        char effect_name[16];
        int lv, mp, mag;
        int consumed = 0;

        if (sscanf(p, "%15s %d %d %d %15s %15s %n",
                   keyword, &lv, &mp, &mag, status_name, effect_name,
                   &consumed) != 6 || strcmp(keyword, "spell") != 0) {
            snprintf(reason, reason_size, "%s:%d: expected a spell line",
                     path, line_number);
            failed = 1;
            break;
        }

        /* %n gives where the name begins; the rest of the line is it. */
        char *name = p + consumed;
        size_t nlen = strlen(name);
        while (nlen > 0 && (name[nlen - 1] == '\n' || name[nlen - 1] == '\r' ||
                            name[nlen - 1] == ' ')) {
            name[nlen - 1] = '\0';
            nlen--;
        }

        if (nlen == 0) {
            snprintf(reason, reason_size, "%s:%d: spell has no name",
                     path, line_number);
            failed = 1;
            break;
        }

        if (lv < 1 || lv > PARTY_MAX_LEVEL || mp < 0 || mag < 0) {
            snprintf(reason, reason_size, "%s:%d: values out of range",
                     path, line_number);
            failed = 1;
            break;
        }

        SpellEffect fn = lookup_effect(effect_name);
        if (fn == NULL) {
            snprintf(reason, reason_size, "%s:%d: unknown effect '%s'",
                     path, line_number, effect_name);
            failed = 1;
            break;
        }

        unsigned flag = STATUS_NONE;
        if (!lookup_status(status_name, &flag)) {
            snprintf(reason, reason_size, "%s:%d: unknown status '%s'",
                     path, line_number, status_name);
            failed = 1;
            break;
        }

        /* The known-spell count relies on the list being sorted. */
        if (count > 0 && lv < parsed[count - 1].level) {
            snprintf(reason, reason_size, "%s:%d: spells must be sorted by level",
                     path, line_number);
            failed = 1;
            break;
        }

        snprintf(parsed[count].name, MAGIC_NAME_LEN, "%s", name);
        parsed[count].level = lv;
        parsed[count].mp_cost = mp;
        parsed[count].magnitude = mag;
        parsed[count].status_flag = flag;
        parsed[count].effect = fn;
        count++;
    }

    fclose(f);

    if (!failed && count == 0) {
        snprintf(reason, reason_size, "%s: no spells in file", path);
        failed = 1;
    }

    if (failed) {
        return 0;
    }

    memcpy(spell_table, parsed, sizeof parsed[0] * (size_t)count);
    spell_count = count;

    snprintf(reason, reason_size, "loaded %d spells from %s", count, path);
    return 1;
}

#define SPELL_COUNT (spell_count)

int magic_spell_count(void)
{
    ensure_spells();
    return SPELL_COUNT;
}

const Spell *magic_spell_at(int index)
{
    ensure_spells();

    if (index < 0 || index >= SPELL_COUNT) {
        return NULL;
    }

    return &spell_table[index];
}

int magic_known_count(int hero_level)
{
    ensure_spells();

    int known = 0;

    for (int i = 0; i < SPELL_COUNT; i++) {
        if (spell_table[i].level <= hero_level) {
            known++;
        }
    }

    return known;
}

int magic_needs_target(const Spell *spell)
{
    if (spell == NULL) {
        return 0;
    }

    return spell->effect == effect_damage ||
           spell->effect == effect_enemy_status;
}

int magic_cast(const Spell *spell, SpellContext *ctx)
{
    if (spell == NULL || ctx == NULL) {
        return 0;
    }

    if (ctx->caster->mp < spell->mp_cost) {
        snprintf(ctx->message, sizeof ctx->message, "Not enough MP.");
        return 0;
    }

    /* A targeted spell with nothing to aim at must not be cast, or the
       effect would dereference a NULL target. */
    if (magic_needs_target(spell) &&
        (ctx->target_hp == NULL || ctx->target_status == NULL)) {
        snprintf(ctx->message, sizeof ctx->message, "Nothing to aim at.");
        return 0;
    }

    ctx->caster->mp -= spell->mp_cost;
    ctx->message[0] = '\0';

    spell->effect(spell, ctx);

    return 1;
}

void magic_tick_status(unsigned *status, int *turns)
{
    if (*status == STATUS_NONE) {
        return;
    }

    if (*turns > 0) {
        (*turns)--;
    }

    if (*turns <= 0) {
        *status = STATUS_NONE;
        *turns = 0;
    }
}
