#include <stdio.h>
#include <stddef.h>
#include "magic.h"
#include "combat_math.h"

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
static const Spell spell_table[] = {
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

#define SPELL_COUNT ((int)(sizeof spell_table / sizeof spell_table[0]))

int magic_spell_count(void)
{
    return SPELL_COUNT;
}

const Spell *magic_spell_at(int index)
{
    if (index < 0 || index >= SPELL_COUNT) {
        return NULL;
    }

    return &spell_table[index];
}

int magic_known_count(int hero_level)
{
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
