#include "test.h"
#include "combat_math.h"
#include "rng.h"
#include "party.h"
#include "battle.h"
#include "entity.h"
#include "inventory.h"
#include "config.h"
#include "dialog.h"
#include "magic.h"
#include "shop.h"
#include "save.h"
#include "world.h"
#include <stdio.h>
#include <string.h>

static void test_base_damage(void)
{
    printf("base damage\n");

    /* An even fight: 10 attack, no weapon, 6 defence -> 10 - 3 = 7. */
    CHECK_EQ(combat_base_damage(10, 0, 6), 7);

    /* A weapon adds its power before defence is subtracted. */
    CHECK_EQ(combat_base_damage(10, 4, 6), 11);

    /* Defence halves, rounding down: 7 / 2 == 3. */
    CHECK_EQ(combat_base_damage(10, 0, 7), 7);

    /* Overwhelming defence still lets 1 through, never 0 or negative. */
    CHECK_EQ(combat_base_damage(5, 0, 40), 1);
    CHECK_EQ(combat_base_damage(1, 0, 999), 1);

    /* Zero attack is still a scratch, not a no-op. */
    CHECK_EQ(combat_base_damage(0, 0, 0), 1);
}

static void test_critical_damage(void)
{
    printf("critical damage\n");

    /* Criticals ignore defence entirely and double what is left. */
    CHECK_EQ(combat_critical_damage(10, 0), 20);
    CHECK_EQ(combat_critical_damage(10, 4), 28);

    /* The floor applies before doubling, so the weakest crit is 2. */
    CHECK_EQ(combat_critical_damage(0, 0), 2);
}

static void test_variance(void)
{
    printf("variance\n");

    /* The midpoint roll leaves damage untouched. */
    CHECK_EQ(combat_apply_variance(100, 12), 100);

    /* The extremes are exactly -12% and +12%. */
    CHECK_EQ(combat_apply_variance(100, 0), 88);
    CHECK_EQ(combat_apply_variance(100, 24), 112);

    /* Small numbers lose the fraction to integer division, which is
       fine -- but must never fall below 1. */
    CHECK_EQ(combat_apply_variance(1, 0), 1);
    CHECK_EQ(combat_apply_variance(2, 0), 2);

    /* Exhaustive: across every legal roll and a wide range of bases,
       the result stays within the advertised band and above zero. */
    for (int base = 1; base <= 500; base++) {
        for (int roll = 0; roll <= 2 * COMBAT_VARIANCE_PERCENT; roll++) {
            int result = combat_apply_variance(base, roll);

            CHECK(result >= 1);
            CHECK(result <= base + (base * COMBAT_VARIANCE_PERCENT) / 100);
            CHECK(result >= base - (base * COMBAT_VARIANCE_PERCENT) / 100
                            || result == 1);
        }
    }
}

static void test_variance_vanishes_on_small_numbers(void)
{
    printf("variance on small numbers (documented limitation)\n");

    /* Below base 9, 12% of the base is less than 1, and integer division
       throws the fraction away -- so early-game damage is completely
       deterministic. This is not a bug in the formula; it is what the
       formula does with small integers, and it is worth pinning down in
       a test so nobody "fixes" it by accident. */
    for (int base = 1; base <= 8; base++) {
        for (int roll = 0; roll <= 2 * COMBAT_VARIANCE_PERCENT; roll++) {
            CHECK_EQ(combat_apply_variance(base, roll), base);
        }
    }

    /* From 9 upwards the band opens up. */
    CHECK_EQ(combat_apply_variance(9, 0), 8);
    CHECK_EQ(combat_apply_variance(9, 24), 10);
}

static void test_hit_chance(void)
{
    printf("hit chance\n");

    /* Equal agility is the baseline 85%. */
    CHECK_EQ(combat_hit_chance(10, 10), 85);

    /* Faster attacker hits more often, slower less. */
    CHECK_EQ(combat_hit_chance(15, 10), 90);
    CHECK_EQ(combat_hit_chance(10, 15), 80);

    /* Clamped at both ends: never a guaranteed hit, never hopeless. */
    CHECK_EQ(combat_hit_chance(200, 0), 99);
    CHECK_EQ(combat_hit_chance(0, 200), 10);
}

static void test_flee_chance(void)
{
    printf("flee chance\n");

    CHECK_EQ(combat_flee_chance(10, 10), 50);

    /* Each point of agility advantage is worth two percent. */
    CHECK_EQ(combat_flee_chance(15, 10), 60);
    CHECK_EQ(combat_flee_chance(10, 15), 40);

    CHECK_EQ(combat_flee_chance(200, 0), 90);
    CHECK_EQ(combat_flee_chance(0, 200), 10);
}

static void test_rng(void)
{
    printf("rng\n");

    Rng a;
    Rng b;

    /* The same seed must give the same sequence -- this is the whole
       reason for carrying our own generator. */
    rng_seed(&a, 12345);
    rng_seed(&b, 12345);
    for (int i = 0; i < 100; i++) {
        CHECK(rng_next(&a) == rng_next(&b));
    }

    /* Different seeds must not immediately agree. */
    rng_seed(&a, 1);
    rng_seed(&b, 2);
    CHECK(rng_next(&a) != rng_next(&b));

    /* Seed 0 is the forbidden state and must be replaced, not accepted. */
    rng_seed(&a, 0);
    CHECK(a.state != 0);
    CHECK(rng_next(&a) != 0);

    /* rng_range stays inside its bounds, every time. */
    rng_seed(&a, 99);
    for (int i = 0; i < 10000; i++) {
        int v = rng_range(&a, 3, 7);
        CHECK(v >= 3 && v <= 7);
    }

    /* A single-value range is legal and always returns that value. */
    CHECK_EQ(rng_range(&a, 5, 5), 5);

    /* An inverted range does not loop forever or read past anything. */
    CHECK_EQ(rng_range(&a, 9, 2), 9);
}

static void test_range_covers_every_value(void)
{
    printf("rng_range coverage\n");

    Rng rng;
    rng_seed(&rng, 2024);

    int seen[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < 5000; i++) {
        seen[rng_range(&rng, 0, 4)]++;
    }

    /* Every value in the range should come up; a generator that never
       returns one end of its range is a classic off-by-one. */
    for (int i = 0; i < 5; i++) {
        CHECK(seen[i] > 0);
    }
}

static void test_rolled_damage_stays_sane(void)
{
    printf("rolled damage\n");

    Rng rng;
    rng_seed(&rng, 777);

    int crits = 0;

    for (int i = 0; i < 20000; i++) {
        int was_crit = 0;
        int dmg = combat_roll_damage(&rng, 12, 3, 8, &was_crit);

        CHECK(dmg >= 1);
        CHECK(dmg <= 100);   /* far above any legitimate result here */

        if (was_crit) {
            crits++;
        }
    }

    /* Criticals are 1 in 32, so roughly 625 of 20000. Allow a wide band
       -- this is checking the rate is plausible, not exact. */
    CHECK(crits > 400);
    CHECK(crits < 900);

    /* Passing NULL for the out-parameter must be allowed. */
    CHECK(combat_roll_damage(&rng, 12, 3, 8, NULL) >= 1);
}


static void test_level_table(void)
{
    printf("level table\n");

    /* The table is indexed by level - 1; out-of-range levels clamp
       rather than reading past the array. */
    CHECK_EQ(party_level_row(1)->hp, 20);
    CHECK_EQ(party_level_row(20)->hp, 240);
    CHECK_EQ(party_level_row(0)->hp, 20);      /* clamps up   */
    CHECK_EQ(party_level_row(-5)->hp, 20);
    CHECK_EQ(party_level_row(99)->hp, 240);    /* clamps down */

    /* Every stat must be non-decreasing as levels rise -- a typo in the
       table that made the hero weaker on level-up would be very hard to
       notice in play. */
    for (int lv = 2; lv <= PARTY_MAX_LEVEL; lv++) {
        const LevelRow *prev = party_level_row(lv - 1);
        const LevelRow *cur = party_level_row(lv);

        CHECK(cur->hp >= prev->hp);
        CHECK(cur->mp >= prev->mp);
        CHECK(cur->attack >= prev->attack);
        CHECK(cur->defense >= prev->defense);
        CHECK(cur->agility >= prev->agility);
    }

    /* The cap must terminate progression, or party_gain_xp would spin. */
    CHECK_EQ(party_level_row(PARTY_MAX_LEVEL)->xp_to_next, 0);
}

static void test_xp_and_levelling(void)
{
    printf("xp and levelling\n");

    Player p;
    memset(&p, 0, sizeof p);
    p.level = 1;
    p.xp = 0;
    p.status = STATUS_NONE;
    p.status_turns = 0;
    for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
    party_apply_level(&p);
    p.hp = p.max_hp;
    p.mp = p.max_mp;

    /* Not enough to level. */
    CHECK_EQ(party_gain_xp(&p, 5), 0);
    CHECK_EQ(p.level, 1);
    CHECK_EQ(p.xp, 5);

    /* Exactly enough tips it over. */
    CHECK_EQ(party_gain_xp(&p, 3), 1);
    CHECK_EQ(p.level, 2);
    CHECK_EQ(p.xp, 0);
    CHECK_EQ(p.max_hp, 26);

    /* A level-up heals to full. */
    CHECK_EQ(p.hp, p.max_hp);

    /* A huge lump of XP can grant several levels at once without
       losing the remainder. */
    p.level = 1;
    p.xp = 0;
    party_apply_level(&p);
    int gained = party_gain_xp(&p, 8 + 18 + 32 + 10);
    CHECK_EQ(gained, 3);
    CHECK_EQ(p.level, 4);
    CHECK_EQ(p.xp, 10);

    /* Zero and negative XP change nothing. */
    CHECK_EQ(party_gain_xp(&p, 0), 0);
    CHECK_EQ(party_gain_xp(&p, -100), 0);
    CHECK_EQ(p.level, 4);

    /* The cap holds, and does not loop forever. */
    p.level = 1;
    p.xp = 0;
    party_apply_level(&p);
    party_gain_xp(&p, 1000000);
    CHECK_EQ(p.level, PARTY_MAX_LEVEL);
}

static void test_battle_setup(void)
{
    printf("battle setup\n");

    Rng rng;
    rng_seed(&rng, 4242);

    /* Area 0 (Grubbin Vale) always produces a runnable battle. */
    for (int i = 0; i < 200; i++) {
        Battle b;
        battle_begin(&b, &rng, 0, 10);

        CHECK(b.enemy_count >= 1);
        CHECK(b.enemy_count <= BATTLE_MAX_ENEMIES);
        CHECK_EQ(b.outcome, BATTLE_ONGOING);
        CHECK_EQ(battle_living_enemies(&b), b.enemy_count);

        for (int e = 0; e < b.enemy_count; e++) {
            CHECK(b.enemies[e].type != NULL);
            CHECK_EQ(b.enemies[e].hp, b.enemies[e].type->max_hp);
            CHECK(b.enemies[e].alive);
        }
    }

    /* A safe area with no roster must not produce a fight to be stuck in. */
    Battle safe;
    battle_begin(&safe, &rng, 2, 10);
    CHECK(safe.outcome != BATTLE_ONGOING);

    /* Out-of-range area indices must be handled, not indexed blindly. */
    Battle bad;
    battle_begin(&bad, &rng, 99, 10);
    CHECK(bad.outcome != BATTLE_ONGOING);
    battle_begin(&bad, &rng, -1, 10);
    CHECK(bad.outcome != BATTLE_ONGOING);
}

static void test_group_size_scales_with_level(void)
{
    printf("group size scales with level\n");

    /* A solo hero acts once per round while every enemy acts, so the
       group cap has to grow with the hero. Measured win rates without
       this cap were 0.0% for a level 1 hero against three enemies. */
    CHECK_EQ(battle_max_group_for_level(1), 1);
    CHECK_EQ(battle_max_group_for_level(2), 1);
    CHECK_EQ(battle_max_group_for_level(3), 2);
    CHECK_EQ(battle_max_group_for_level(5), 2);
    CHECK_EQ(battle_max_group_for_level(6), 3);
    CHECK_EQ(battle_max_group_for_level(20), 3);

    /* Nonsense levels must not produce a group of zero, which would be
       an unwinnable, unloseable battle. */
    CHECK(battle_max_group_for_level(0) >= 1);
    CHECK(battle_max_group_for_level(-5) >= 1);

    /* And the cap is actually honoured when building a battle. */
    Rng rng;
    rng_seed(&rng, 606);
    for (int i = 0; i < 300; i++) {
        Battle b;
        battle_begin(&b, &rng, 0, 1);
        CHECK(b.enemy_count == 1);
    }
    for (int i = 0; i < 300; i++) {
        Battle b;
        battle_begin(&b, &rng, 0, 4);
        CHECK(b.enemy_count >= 1 && b.enemy_count <= 2);
    }
}

static void test_battle_terminates(void)
{
    printf("battles always end\n");

    Rng rng;
    rng_seed(&rng, 31337);

    /* Every battle must reach a conclusion in a sane number of rounds.
       A formula that could do zero damage would hang here forever, which
       is exactly the bug the damage floor in Chapter 15 prevents. */
    for (int trial = 0; trial < 300; trial++) {
        Player p;
        memset(&p, 0, sizeof p);
        p.level = 5;
        p.xp = 0;
        p.status = STATUS_NONE;
        p.status_turns = 0;
        for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
        party_apply_level(&p);
        p.hp = p.max_hp;
        p.mp = p.max_mp;

        Battle b;
        battle_begin(&b, &rng, 0, 10);

        int rounds = 0;
        while (b.outcome == BATTLE_ONGOING && rounds < 500) {
            battle_round(&b, &p, &rng, 0, 100);
            rounds++;
        }

        CHECK(b.outcome != BATTLE_ONGOING);
        CHECK(rounds < 500);
        CHECK(p.hp >= 0);

        /* Rewards only accrue on a win, and only from dead enemies. */
        if (b.outcome == BATTLE_WON) {
            CHECK_EQ(battle_living_enemies(&b), 0);
            CHECK(b.xp_reward > 0);
        }
    }
}

static void test_fleeing_terminates(void)
{
    printf("fleeing always resolves\n");

    Rng rng;
    rng_seed(&rng, 8080);

    for (int trial = 0; trial < 200; trial++) {
        Player p;
        memset(&p, 0, sizeof p);
        p.level = 3;
        p.xp = 0;
        p.status = STATUS_NONE;
        p.status_turns = 0;
        for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
        party_apply_level(&p);
        p.hp = p.max_hp;
        p.mp = p.max_mp;

        Battle b;
        battle_begin(&b, &rng, 0, 10);

        int rounds = 0;
        while (b.outcome == BATTLE_ONGOING && rounds < 500) {
            battle_round(&b, &p, &rng, 1, 100);
            rounds++;
        }

        CHECK(b.outcome != BATTLE_ONGOING);
        CHECK(rounds < 500);
    }
}


static void test_inventory_growth(void)
{
    printf("inventory growth\n");

    Inventory inv;
    inventory_init(&inv);

    /* An empty inventory owns nothing at all. */
    CHECK(inv.stacks == NULL);
    CHECK_EQ(inv.count, 0);
    CHECK_EQ(inv.capacity, 0);

    /* Adding the first item allocates. */
    CHECK(inventory_add(&inv, ITEM_BREAD_RATION, 1));
    CHECK(inv.stacks != NULL);
    CHECK_EQ(inv.count, 1);
    CHECK(inv.capacity >= 1);

    /* Adding the same item stacks instead of growing. */
    CHECK(inventory_add(&inv, ITEM_BREAD_RATION, 4));
    CHECK_EQ(inv.count, 1);
    CHECK_EQ(inventory_count_of(&inv, ITEM_BREAD_RATION), 5);

    /* Distinct items each take a slot, and capacity must keep up. */
    CHECK(inventory_add(&inv, ITEM_HERB_POULTICE, 1));
    CHECK(inventory_add(&inv, ITEM_BOTTLED_VIM, 1));
    CHECK(inventory_add(&inv, ITEM_BELL_OF_MILD_ALARM, 1));
    CHECK(inventory_add(&inv, ITEM_GUILD_COMMISSION, 1));
    CHECK(inventory_add(&inv, ITEM_BRASS_KEY, 1));
    CHECK_EQ(inv.count, 6);
    CHECK(inv.capacity >= inv.count);

    /* Every stack survived the reallocations intact -- this is the
       check that catches a realloc that lost or corrupted data. */
    CHECK_EQ(inventory_count_of(&inv, ITEM_BREAD_RATION), 5);
    CHECK_EQ(inventory_count_of(&inv, ITEM_HERB_POULTICE), 1);
    CHECK_EQ(inventory_count_of(&inv, ITEM_BRASS_KEY), 1);

    /* Nonsense input is rejected rather than stored. */
    CHECK(!inventory_add(&inv, ITEM_BREAD_RATION, 0));
    CHECK(!inventory_add(&inv, ITEM_BREAD_RATION, -3));
    CHECK(!inventory_add(&inv, (ItemId)999, 1));
    CHECK(!inventory_add(&inv, (ItemId)-1, 1));
    CHECK_EQ(inv.count, 6);

    inventory_free(&inv);
    CHECK(inv.stacks == NULL);
    CHECK_EQ(inv.count, 0);
}

static void test_inventory_removal(void)
{
    printf("inventory removal\n");

    Inventory inv;
    inventory_init(&inv);

    inventory_add(&inv, ITEM_BREAD_RATION, 2);
    inventory_add(&inv, ITEM_HERB_POULTICE, 1);

    CHECK(inventory_remove_one(&inv, ITEM_BREAD_RATION));
    CHECK_EQ(inventory_count_of(&inv, ITEM_BREAD_RATION), 1);
    CHECK_EQ(inv.count, 2);

    /* Emptying a stack drops the slot entirely. */
    CHECK(inventory_remove_one(&inv, ITEM_BREAD_RATION));
    CHECK_EQ(inventory_count_of(&inv, ITEM_BREAD_RATION), 0);
    CHECK_EQ(inv.count, 1);

    /* The other stack must still be intact after the gap was closed. */
    CHECK_EQ(inventory_count_of(&inv, ITEM_HERB_POULTICE), 1);

    /* Removing what you do not have fails cleanly. */
    CHECK(!inventory_remove_one(&inv, ITEM_BREAD_RATION));
    CHECK(!inventory_remove_one(&inv, ITEM_BRASS_KEY));

    inventory_free(&inv);

    /* Freeing twice must be safe, since inventory_free clears the
       pointer -- the discipline from Chapter 10. */
    inventory_free(&inv);
}

static void test_inventory_many_items(void)
{
    printf("inventory stress\n");

    Inventory inv;
    inventory_init(&inv);

    /* Repeatedly fill and empty, which exercises grow paths and the
       swap-with-last removal, and would surface a leak under ASan. */
    for (int round = 0; round < 50; round++) {
        for (int id = 0; id < ITEM_TYPE_COUNT; id++) {
            inventory_add(&inv, (ItemId)id, 3);
        }
        CHECK_EQ(inv.count, ITEM_TYPE_COUNT);

        for (int id = 0; id < ITEM_TYPE_COUNT; id++) {
            for (int k = 0; k < 3; k++) {
                CHECK(inventory_remove_one(&inv, (ItemId)id));
            }
        }
        CHECK_EQ(inv.count, 0);
    }

    inventory_free(&inv);
}

static void write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return;
    }
    fputs(text, f);
    fclose(f);
}

static void test_config(void)
{
    printf("config parsing\n");

    Config c;

    /* Defaults stand on their own. */
    config_defaults(&c);
    CHECK_EQ(c.encounters, 1);
    CHECK_EQ(c.enemy_damage_pct, 100);
    CHECK_EQ(c.xp_rate_pct, 100);
    CHECK_EQ(c.gold_loss_pct, 0);

    /* A missing file is not an error and changes nothing. */
    CHECK_EQ(config_load(&c, "definitely_not_a_file.txt"), 0);
    CHECK_EQ(c.enemy_damage_pct, 100);

    /* A well-formed file overrides. */
    write_file("/tmp/cfg_ok.txt",
               "# comment\n"
               "\n"
               "encounters = off\n"
               "  enemy_damage =  50 \n"
               "xp_rate=200\n"
               "gold_loss_on_death = 25\n");
    config_defaults(&c);
    CHECK_EQ(config_load(&c, "/tmp/cfg_ok.txt"), 1);
    CHECK_EQ(c.encounters, 0);
    CHECK_EQ(c.enemy_damage_pct, 50);
    CHECK_EQ(c.xp_rate_pct, 200);
    CHECK_EQ(c.gold_loss_pct, 25);

    /* A partial file leaves everything else at its default. */
    write_file("/tmp/cfg_partial.txt", "xp_rate = 150\n");
    config_defaults(&c);
    config_load(&c, "/tmp/cfg_partial.txt");
    CHECK_EQ(c.xp_rate_pct, 150);
    CHECK_EQ(c.encounters, 1);
    CHECK_EQ(c.enemy_damage_pct, 100);

    /* Garbage is rejected per-line; good lines around it still apply. */
    write_file("/tmp/cfg_bad.txt",
               "encounters = maybe\n"
               "enemy_damage = banana\n"
               "xp_rate = 99999\n"
               "gold_loss_on_death = -5\n"
               "no_equals_sign_here\n"
               "unknown_key = 3\n"
               "enemy_damage = 75\n");
    config_defaults(&c);
    config_load(&c, "/tmp/cfg_bad.txt");
    CHECK_EQ(c.encounters, 1);          /* rejected, kept default */
    CHECK_EQ(c.xp_rate_pct, 100);       /* out of range, rejected */
    CHECK_EQ(c.gold_loss_pct, 0);       /* negative, rejected     */
    CHECK_EQ(c.enemy_damage_pct, 75);   /* the valid line applied */

    /* Boolean spellings all work. */
    const char *yeses[] = { "on", "true", "yes", "1" };
    const char *nos[] = { "off", "false", "no", "0" };
    for (int i = 0; i < 4; i++) {
        char text[64];
        snprintf(text, sizeof text, "encounters = %s\n", yeses[i]);
        write_file("/tmp/cfg_bool.txt", text);
        config_defaults(&c);
        config_load(&c, "/tmp/cfg_bool.txt");
        CHECK_EQ(c.encounters, 1);

        snprintf(text, sizeof text, "encounters = %s\n", nos[i]);
        write_file("/tmp/cfg_bool.txt", text);
        config_defaults(&c);
        config_load(&c, "/tmp/cfg_bool.txt");
        CHECK_EQ(c.encounters, 0);
    }
}


static void test_dialog_paging_loses_nothing(void)
{
    printf("dialog paging preserves all text\n");

    /* Every speech actually shipped in the game. If any of these ever
       stops round-tripping, an NPC is being cut off mid-sentence -- which
       is exactly the bug a playtest caught after this course first
       shipped chapter 12 with a hard four-line cap and no paging. */
    static const char *speeches[] = {
        "Bed's six gold. Sleep fixes most things, in my experience. "
        "Not everything. Most.",
        "Four pieces. Scattered north, south, and -- the other two "
        "directions. I had it written down.",
        "The Guild is prepared to offer forty gold up front and a "
        "completion bonus that I'd rather not describe as generous. "
        "Eleven others declined.",
        "Something got into the turnips. Not a fox. Foxes don't do "
        "that to a fence.",
        "Sorry -- is this about the vault inventory? It's not ready. "
        "It's been eleven years and it's not ready.",
        "Everything's twice what it was. That's not me gouging. "
        "That's the roads."
    };
    int speech_count = (int)(sizeof speeches / sizeof speeches[0]);

    for (int s = 0; s < speech_count; s++) {
        const char *text = speeches[s];
        size_t source_pos = 0;
        size_t source_len = strlen(text);
        int pages = 0;

        Dialog d;
        dialog_start(&d, text, 18);

        do {
            pages++;
            CHECK(pages <= 20);          /* never loop forever */

            for (int i = 0; i < d.line_count; i++) {
                size_t line_len = strlen(d.lines[i]);

                CHECK(line_len <= 18);   /* never exceeds the width */
                CHECK(line_len < DIALOG_LINE_LEN);

                /* Compare ignoring whitespace on both sides: wrapping
                   legitimately moves spaces around, but must never lose,
                   duplicate or reorder a printable character. */
                for (size_t c = 0; c < line_len; c++) {
                    if (d.lines[i][c] == ' ') {
                        continue;
                    }
                    while (source_pos < source_len && text[source_pos] == ' ') {
                        source_pos++;
                    }
                    CHECK(source_pos < source_len);
                    if (source_pos < source_len) {
                        CHECK(text[source_pos] == d.lines[i][c]);
                    }
                    source_pos++;
                }
            }
        } while (dialog_advance(&d));

        /* Only trailing spaces may remain: nothing was dropped. */
        while (source_pos < source_len && text[source_pos] == ' ') {
            source_pos++;
        }
        CHECK_EQ((int)source_pos, (int)source_len);
        CHECK(pages >= 1);
        CHECK(pages >= 1);
    }

    /* Edge cases must not hang or misreport. */
    Dialog d;
    dialog_start(&d, "", 18);
    CHECK_EQ(d.line_count, 0);
    CHECK_EQ(d.more, 0);
    CHECK_EQ(dialog_advance(&d), 0);

    dialog_start(&d, NULL, 18);
    CHECK_EQ(d.line_count, 0);
    CHECK_EQ(dialog_advance(&d), 0);

    dialog_start(&d, "short", 18);
    CHECK_EQ(d.line_count, 1);
    CHECK_EQ(d.more, 0);

    /* A single word longer than the whole box must still terminate. */
    dialog_start(&d, "Supercalifragilisticexpialidociousness rides", 10);
    int guard = 0;
    do {
        guard++;
    } while (dialog_advance(&d) && guard < 50);
    CHECK(guard < 50);
}


static void test_spell_table(void)
{
    printf("spell table\n");

    CHECK_EQ(magic_spell_count(), 13);

    /* Ordered by level, which is what lets magic_known_count just count
       a prefix instead of searching. */
    for (int i = 1; i < magic_spell_count(); i++) {
        CHECK(magic_spell_at(i)->level >= magic_spell_at(i - 1)->level);
    }

    /* Every row must have an effect, or casting it dereferences NULL. */
    for (int i = 0; i < magic_spell_count(); i++) {
        const Spell *s = magic_spell_at(i);
        CHECK(s->effect != NULL);
        CHECK(s->name != NULL);
        CHECK(s->mp_cost > 0 || s->level == 0);
    }

    /* Out of range must be NULL, not a wild pointer. */
    CHECK(magic_spell_at(-1) == NULL);
    CHECK(magic_spell_at(magic_spell_count()) == NULL);
    CHECK(magic_spell_at(9999) == NULL);

    /* Known counts follow the table. */
    CHECK_EQ(magic_known_count(1), 0);
    CHECK_EQ(magic_known_count(2), 1);
    CHECK_EQ(magic_known_count(3), 2);
    CHECK_EQ(magic_known_count(20), 13);
    CHECK_EQ(magic_known_count(0), 0);
}

static void test_spell_casting(void)
{
    printf("spell casting\n");

    Rng rng;
    rng_seed(&rng, 1234);

    Player p;
    memset(&p, 0, sizeof p);
    p.level = 10;
    p.xp = 0;
    p.status = STATUS_NONE;
    p.status_turns = 0;
    for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
    party_apply_level(&p);
    p.hp = 10;
    p.mp = p.max_mp;

    SpellContext ctx;
    ctx.caster = &p;
    ctx.rng = &rng;
    ctx.target_hp = NULL;
    ctx.target_status = NULL;
    ctx.target_turns = NULL;
    ctx.message[0] = '\0';

    /* Mend heals and costs MP. */
    const Spell *mend = magic_spell_at(0);
    int mp_before = p.mp;
    CHECK(magic_cast(mend, &ctx));
    CHECK(p.hp > 10);
    CHECK_EQ(p.mp, mp_before - mend->mp_cost);
    CHECK(ctx.message[0] != '\0');

    /* Healing never overshoots the maximum. */
    p.hp = p.max_hp - 1;
    magic_cast(mend, &ctx);
    CHECK_EQ(p.hp, p.max_hp);

    /* Not enough MP is refused, and costs nothing. */
    p.mp = 0;
    int hp_before = p.hp;
    CHECK(!magic_cast(mend, &ctx));
    CHECK_EQ(p.mp, 0);
    CHECK_EQ(p.hp, hp_before);

    /* A targeted spell with no target is refused rather than crashing --
       this is the check that stops effect_damage dereferencing NULL. */
    p.mp = p.max_mp;
    const Spell *ember = magic_spell_at(1);
    CHECK(magic_needs_target(ember));
    CHECK(!magic_cast(ember, &ctx));
    CHECK_EQ(p.mp, p.max_mp);

    /* With a target it works and spends MP. */
    int enemy_hp = 100;
    unsigned enemy_status = STATUS_NONE;
    int enemy_turns = 0;
    ctx.target_hp = &enemy_hp;
    ctx.target_status = &enemy_status;
    ctx.target_turns = &enemy_turns;
    CHECK(magic_cast(ember, &ctx));
    CHECK(enemy_hp < 100);

    /* A status spell sets exactly its own bit and a duration. */
    p.mp = p.max_mp;
    const Spell *dull = magic_spell_at(2);
    CHECK(magic_cast(dull, &ctx));
    CHECK(enemy_status & STATUS_DULLED);
    CHECK(!(enemy_status & STATUS_ASLEEP));
    CHECK(enemy_turns > 0);

    /* NULL spell must not crash. */
    CHECK(!magic_cast(NULL, &ctx));
}

static void test_status_flags(void)
{
    printf("status bit flags\n");

    unsigned status = STATUS_NONE;
    int turns = 0;

    /* Several statuses coexist in one unsigned. */
    status |= STATUS_STONESKIN;
    status |= STATUS_HASTENED;
    CHECK(status & STATUS_STONESKIN);
    CHECK(status & STATUS_HASTENED);
    CHECK(!(status & STATUS_ASLEEP));

    /* Clearing one leaves the others alone. */
    status &= ~STATUS_STONESKIN;
    CHECK(!(status & STATUS_STONESKIN));
    CHECK(status & STATUS_HASTENED);

    /* Every flag is a distinct bit -- a duplicate would make two
       different statuses indistinguishable. */
    unsigned all[] = { STATUS_STONESKIN, STATUS_HASTENED, STATUS_WARDED,
                       STATUS_ASLEEP, STATUS_DULLED };
    for (int i = 0; i < 5; i++) {
        CHECK(all[i] != 0);
        for (int j = i + 1; j < 5; j++) {
            CHECK((all[i] & all[j]) == 0);
        }
    }

    /* Timers count down and clear the flags when they expire. */
    status = STATUS_STONESKIN;
    turns = 2;
    magic_tick_status(&status, &turns);
    CHECK(status & STATUS_STONESKIN);
    magic_tick_status(&status, &turns);
    CHECK_EQ(status, STATUS_NONE);
    CHECK_EQ(turns, 0);

    /* Ticking an empty status is harmless and does not underflow. */
    magic_tick_status(&status, &turns);
    CHECK_EQ(status, STATUS_NONE);
    CHECK_EQ(turns, 0);
}

static void test_equipment(void)
{
    printf("equipment\n");

    Player p;
    memset(&p, 0, sizeof p);
    p.level = 1;
    p.xp = 0;
    p.status = STATUS_NONE;
    p.status_turns = 0;
    for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
    party_apply_level(&p);

    /* Starting gear: Chair Leg +2 ATK, Work Clothes +1 DEF. */
    CHECK_EQ(entity_attack(&p), p.attack + 2);
    CHECK_EQ(entity_defense(&p), p.defense + 1);
    CHECK_EQ(entity_agility(&p), p.agility);

    /* A better weapon raises attack and nothing else. */
    int def_before = entity_defense(&p);
    p.equipped[SLOT_WEAPON] = 1;             /* Bronze Shortsword +6 */
    CHECK_EQ(entity_attack(&p), p.attack + 6);
    CHECK_EQ(entity_defense(&p), def_before);

    /* An accessory with a downside applies both halves. */
    p.equipped[SLOT_ACCESSORY] = 2;          /* Ledger-Weight +5 DEF -2 AGI */
    CHECK_EQ(entity_defense(&p), p.defense + 1 + 5);
    CHECK_EQ(entity_agility(&p), p.agility - 2);

    /* Agility can never reach zero, or turn order and hit chance break. */
    p.agility = 1;
    CHECK(entity_agility(&p) >= 1);

    /* Status multipliers stack on top of gear. */
    p.agility = 10;
    p.equipped[SLOT_ACCESSORY] = 0;
    int plain = entity_agility(&p);
    p.status = STATUS_HASTENED;
    CHECK(entity_agility(&p) > plain);
    p.status = STATUS_NONE;

    /* Out-of-range gear indices must be reported, not indexed. */
    CHECK(gear_at(SLOT_WEAPON, -1) == NULL);
    CHECK(gear_at(SLOT_WEAPON, 9999) == NULL);
    CHECK(gear_at((GearSlot)99, 0) == NULL);
    CHECK_EQ(gear_count((GearSlot)99), 0);

    for (int s = 0; s < SLOT_COUNT; s++) {
        CHECK(gear_count((GearSlot)s) > 0);
        for (int i = 0; i < gear_count((GearSlot)s); i++) {
            CHECK(gear_at((GearSlot)s, i) != NULL);
            CHECK(gear_at((GearSlot)s, i)->name != NULL);
        }
    }
}


static void test_enemy_only_turn(void)
{
    printf("enemy-only turn (used when the hero casts)\n");

    Rng rng;
    rng_seed(&rng, 5150);

    for (int trial = 0; trial < 200; trial++) {
        Player p;
        memset(&p, 0, sizeof p);
        p.level = 8;
        p.xp = 0;
        p.status = STATUS_NONE;
        p.status_turns = 0;
        for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
        party_apply_level(&p);
        p.hp = p.max_hp;
        p.mp = p.max_mp;

        Battle b;
        battle_begin(&b, &rng, 0, p.level);
        if (b.outcome != BATTLE_ONGOING) continue;

        int enemies_before = battle_living_enemies(&b);
        int hp_before = p.hp;

        battle_enemy_turn(&b, &p, &rng, 100);

        /* Enemies act; the hero does not, so no enemy may die. */
        CHECK_EQ(battle_living_enemies(&b), enemies_before);
        CHECK(p.hp <= hp_before);
        CHECK(p.hp >= 0);

        /* And it must terminate, exactly like a full round. */
        int rounds = 0;
        while (b.outcome == BATTLE_ONGOING && rounds < 500) {
            battle_enemy_turn(&b, &p, &rng, 100);
            rounds++;
        }
        CHECK(rounds < 500);
        CHECK_EQ(b.outcome, BATTLE_LOST);   /* hero never fights back */
    }

    /* A sleeping enemy does nothing at all. */
    Player p;
    memset(&p, 0, sizeof p);
    p.level = 8; p.xp = 0; p.status = STATUS_NONE; p.status_turns = 0;
    for (int s = 0; s < SLOT_COUNT; s++) p.equipped[s] = 0;
    party_apply_level(&p);
    p.hp = p.max_hp;

    Battle b;
    battle_begin(&b, &rng, 0, 1);
    if (b.outcome == BATTLE_ONGOING) {
        for (int i = 0; i < b.enemy_count; i++) {
            b.enemies[i].status = STATUS_ASLEEP;
            b.enemies[i].status_turns = 5;
        }
        int hp = p.hp;
        battle_enemy_turn(&b, &p, &rng, 100);
        CHECK_EQ(p.hp, hp);
    }
}


static void make_hero(Player *p, int level, int gold)
{
    memset(p, 0, sizeof *p);
    p->level = level;
    p->xp = 0;
    p->gold = gold;
    p->status = STATUS_NONE;
    p->status_turns = 0;
    for (int s = 0; s < SLOT_COUNT; s++) p->equipped[s] = 0;
    party_apply_level(p);
    p->hp = p->max_hp;
    p->mp = p->max_mp;
}

static void test_shop_lookup(void)
{
    printf("shop lookup\n");

    /* Grubbin Vale and Castle Hollis trade; the barrow does not. */
    CHECK(shop_for_area(0) != NULL);
    CHECK(shop_for_area(1) == NULL);
    CHECK(shop_for_area(2) != NULL);

    /* Out of range must be NULL rather than an indexed read. */
    CHECK(shop_for_area(-1) == NULL);
    CHECK(shop_for_area(99) == NULL);

    /* Every stocked line must name something real and cost something. */
    for (int a = 0; a < 3; a++) {
        const Shop *s = shop_for_area(a);
        if (s == NULL) continue;

        CHECK(s->stock_count <= SHOP_MAX_STOCK);
        for (int i = 0; i < s->stock_count; i++) {
            const char *name = shop_entry_name(&s->stock[i]);
            CHECK(name != NULL);
            CHECK(strcmp(name, "(nothing)") != 0);
            CHECK(shop_entry_price(&s->stock[i]) > 0);
        }
    }

    /* A NULL entry is handled, not dereferenced. */
    CHECK(shop_entry_price(NULL) == 0);
    CHECK(strcmp(shop_entry_name(NULL), "(nothing)") == 0);
}

static void test_shop_buying(void)
{
    printf("shop buying\n");

    const Shop *shop = shop_for_area(0);
    char reason[48];

    Player p;
    Inventory inv;

    /* Too poor: refused, and charged nothing. */
    make_hero(&p, 1, 0);
    inventory_init(&inv);
    CHECK(!shop_buy(&shop->stock[0], &p, &inv, reason, sizeof reason));
    CHECK_EQ(p.gold, 0);
    CHECK_EQ(inv.count, 0);
    CHECK(reason[0] != '\0');

    /* Exactly enough: succeeds and spends exactly the price. */
    int price = shop_entry_price(&shop->stock[0]);
    make_hero(&p, 1, price);
    CHECK(shop_buy(&shop->stock[0], &p, &inv, reason, sizeof reason));
    CHECK_EQ(p.gold, 0);

    /* One gold short: refused. */
    make_hero(&p, 1, price - 1);
    p.equipped[SLOT_WEAPON] = 0;
    CHECK(!shop_buy(&shop->stock[0], &p, &inv, reason, sizeof reason));
    CHECK_EQ(p.gold, price - 1);

    /* Buying gear equips it and changes the effective stat. */
    make_hero(&p, 1, 1000);
    int atk_before = entity_attack(&p);
    CHECK(shop_buy(&shop->stock[0], &p, &inv, reason, sizeof reason));
    CHECK(entity_attack(&p) > atk_before);

    /* Buying the same gear twice is refused rather than charged for. */
    int gold_before = p.gold;
    CHECK(!shop_buy(&shop->stock[0], &p, &inv, reason, sizeof reason));
    CHECK_EQ(p.gold, gold_before);

    /* Buying a consumable puts it in the bag and charges once. */
    make_hero(&p, 1, 1000);
    inventory_free(&inv);
    inventory_init(&inv);

    const StockEntry *ration = NULL;
    for (int i = 0; i < shop->stock_count; i++) {
        if (shop->stock[i].kind == STOCK_ITEM) {
            ration = &shop->stock[i];
            break;
        }
    }
    CHECK(ration != NULL);

    if (ration != NULL) {
        int cost = shop_entry_price(ration);
        int before = p.gold;
        CHECK(shop_buy(ration, &p, &inv, reason, sizeof reason));
        CHECK_EQ(p.gold, before - cost);
        CHECK_EQ(inventory_count_of(&inv, (ItemId)ration->index), 1);

        /* Buying again stacks rather than adding a second row. */
        CHECK(shop_buy(ration, &p, &inv, reason, sizeof reason));
        CHECK_EQ(inventory_count_of(&inv, (ItemId)ration->index), 2);
        CHECK_EQ(inv.count, 1);
    }

    /* NULL arguments must be refused, not crashed on. */
    CHECK(!shop_buy(NULL, &p, &inv, reason, sizeof reason));

    inventory_free(&inv);
}

static void test_inn(void)
{
    printf("inn\n");

    const Shop *shop = shop_for_area(0);
    char reason[48];
    Player p;

    /* Hurt and able to pay: restored, and charged. */
    make_hero(&p, 5, 100);
    p.hp = 1;
    p.mp = 0;
    CHECK(shop_rest(shop, &p, reason, sizeof reason));
    CHECK_EQ(p.hp, p.max_hp);
    CHECK_EQ(p.mp, p.max_mp);
    CHECK_EQ(p.gold, 100 - shop->inn_cost);

    /* Already rested: refused rather than charged for nothing. */
    int gold = p.gold;
    CHECK(!shop_rest(shop, &p, reason, sizeof reason));
    CHECK_EQ(p.gold, gold);

    /* Too poor: refused, and not healed. */
    make_hero(&p, 5, 0);
    p.hp = 1;
    CHECK(!shop_rest(shop, &p, reason, sizeof reason));
    CHECK_EQ(p.hp, 1);
    CHECK_EQ(p.gold, 0);

    /* An area with no inn. */
    CHECK(!shop_rest(shop_for_area(1), &p, reason, sizeof reason));
    CHECK(!shop_rest(NULL, &p, reason, sizeof reason));
}

static void test_gold_never_negative(void)
{
    printf("gold never goes negative\n");

    Rng rng;
    rng_seed(&rng, 2718);

    char reason[48];
    Inventory inv;
    inventory_init(&inv);

    /* Hammer every shop line at every plausible purse size. Gold must
       never end up below zero, whatever the sequence. */
    for (int trial = 0; trial < 400; trial++) {
        Player p;
        make_hero(&p, 1, rng_range(&rng, 0, 200));

        for (int step = 0; step < 20; step++) {
            const Shop *shop = shop_for_area(rng_range(&rng, 0, 2));
            if (shop == NULL) continue;

            int rows = shop->stock_count + (shop->inn_cost > 0 ? 1 : 0);
            int pick = rng_range(&rng, 0, rows - 1);

            if (pick < shop->stock_count) {
                shop_buy(&shop->stock[pick], &p, &inv, reason, sizeof reason);
            } else {
                p.hp = 1;
                shop_rest(shop, &p, reason, sizeof reason);
            }

            CHECK(p.gold >= 0);
        }
    }

    inventory_free(&inv);
}


static void write_text(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");
    if (f != NULL) {
        fputs(text, f);
        fclose(f);
    }
}

static int try_save_load(const char *text, char *reason, size_t rsize)
{
    write_text("/tmp/sav_test.txt", text);

    SaveData d;
    Inventory inv;
    inventory_init(&inv);

    int ok = save_read("/tmp/sav_test.txt", &d, &inv, reason, rsize);
    inventory_free(&inv);
    return ok;
}

static void test_save_round_trip(void)
{
    printf("save round trip\n");

    SaveData out;
    memset(&out, 0, sizeof out);
    out.version = SAVE_VERSION;
    snprintf(out.name, sizeof out.name, "Wick");
    out.area = 2; out.x = 7; out.y = 9;
    out.level = 5; out.xp = 41; out.gold = 133;
    out.hp = 30; out.mp = 6;
    out.equipped[SLOT_WEAPON] = 2;
    out.equipped[SLOT_ARMOUR] = 1;
    out.equipped[SLOT_ACCESSORY] = 0;
    out.rng_state = 987654321u;

    Inventory inv;
    inventory_init(&inv);
    inventory_add(&inv, ITEM_BREAD_RATION, 3);
    inventory_add(&inv, ITEM_BRASS_KEY, 1);

    char reason[96];
    CHECK(save_write("/tmp/sav_test.txt", &out, &inv, reason, sizeof reason));

    SaveData in;
    Inventory in_inv;
    inventory_init(&in_inv);
    CHECK(save_read("/tmp/sav_test.txt", &in, &in_inv, reason, sizeof reason));

    /* Every field must survive the trip unchanged. */
    CHECK_EQ(in.area, out.area);
    CHECK_EQ(in.x, out.x);
    CHECK_EQ(in.y, out.y);
    CHECK_EQ(in.level, out.level);
    CHECK_EQ(in.xp, out.xp);
    CHECK_EQ(in.gold, out.gold);
    CHECK_EQ(in.hp, out.hp);
    CHECK_EQ(in.mp, out.mp);
    CHECK_EQ((int)in.rng_state, (int)out.rng_state);
    CHECK(strcmp(in.name, out.name) == 0);
    for (int s = 0; s < SLOT_COUNT; s++) {
        CHECK_EQ(in.equipped[s], out.equipped[s]);
    }

    /* Including the inventory, stacks and all. */
    CHECK_EQ(in_inv.count, 2);
    CHECK_EQ(inventory_count_of(&in_inv, ITEM_BREAD_RATION), 3);
    CHECK_EQ(inventory_count_of(&in_inv, ITEM_BRASS_KEY), 1);

    inventory_free(&inv);
    inventory_free(&in_inv);
}

static void test_save_refuses_bad_files(void)
{
    printf("save refuses every malformed file\n");

    char reason[96];

    /* A well-formed save loads. */
    CHECK(try_save_load(
        "version = 1\nname = Wick\narea = 0\nx = 2\ny = 2\nlevel = 3\n"
        "xp = 5\ngold = 40\nhp = 20\nmp = 4\nweapon = 0\narmour = 0\n"
        "accessory = 0\nrng = 12345\n", reason, sizeof reason));

    /* Everything else must be refused -- and must say why. */
    static const char *bad[] = {
        "version = 99\nname = W\nlevel = 2\n",          /* wrong version   */
        "name = Wick\nlevel = 3\n",                      /* no version      */
        "version = 1\nname = Wick\n",                    /* no level        */
        "version = 1\nlevel = 3\n",                      /* no name         */
        "version = 1\nname = W\nlevel = 999\n",         /* level too high  */
        "version = 1\nname = W\nlevel = 0\n",           /* level too low   */
        "version = 1\nname = W\nlevel = 3\ngold = -5\n",
        "version = 1\nname = W\nlevel = 3\ngold = banana\n",
        "version = 1\nname = W\nlevel = 3\ngold = 50kg\n",
        "version = 1\nname = W\nlevel = 3\narea = 77\n",
        "version = 1\nname = W\nlevel = 3\nweapon = 99\n",
        "version = 1\nname = W\nlevel = 3\nwings = 4\n",
        "version = 1\nname = W\nlevel = 3\ngarbage\n",
        "version = 1\nname = W\nlevel = 3\nitem = 99 1\n",
        "version = 1\nname = W\nlevel = 3\nitem = 0 -4\n",
        "version = 1\nname = W\nlevel = 3\nitem = 0\n",
        ""
    };
    int bad_count = (int)(sizeof bad / sizeof bad[0]);

    for (int i = 0; i < bad_count; i++) {
        reason[0] = '\0';
        CHECK(!try_save_load(bad[i], reason, sizeof reason));
        CHECK(reason[0] != '\0');   /* a refusal must explain itself */
    }

    /* A missing file is refused, not treated as an empty save. */
    SaveData d;
    Inventory inv;
    inventory_init(&inv);
    CHECK(!save_read("/tmp/definitely_absent_save.txt", &d, &inv,
                     reason, sizeof reason));
    CHECK(!save_exists("/tmp/definitely_absent_save.txt"));
    inventory_free(&inv);
}

static void test_load_never_partially_applies(void)
{
    printf("a refused load changes nothing\n");

    /* Fill a target with known values, attempt to load rubbish over it,
       and prove not one byte moved. */
    SaveData target;
    memset(&target, 0, sizeof target);
    target.level = 7;
    target.gold = 500;
    target.area = 1;
    snprintf(target.name, sizeof target.name, "Original");

    SaveData before = target;

    Inventory inv;
    inventory_init(&inv);
    inventory_add(&inv, ITEM_HERB_POULTICE, 2);

    char reason[96];

    /* This file is valid right up until its last line. */
    write_text("/tmp/sav_partial.txt",
               "version = 1\nname = Impostor\nlevel = 12\ngold = 9999\n"
               "area = 0\nwings = 3\n");

    CHECK(!save_read("/tmp/sav_partial.txt", &target, &inv,
                     reason, sizeof reason));

    CHECK_EQ(target.level, before.level);
    CHECK_EQ(target.gold, before.gold);
    CHECK_EQ(target.area, before.area);
    CHECK(strcmp(target.name, "Original") == 0);

    /* And the inventory it was handed is still intact. */
    CHECK_EQ(inventory_count_of(&inv, ITEM_HERB_POULTICE), 2);

    inventory_free(&inv);
}


static void test_data_tables_load(void)
{
    printf("data-driven tables\n");

    char reason[96];

    /* The shipped files must load, and must agree with what the game
       had built in -- otherwise moving the tables to disk silently
       changed the balance. */
    CHECK(party_load_levels("assets/levels.txt", reason, sizeof reason));
    CHECK_EQ(party_level_row(1)->hp, 20);
    CHECK_EQ(party_level_row(1)->xp_to_next, 8);
    CHECK_EQ(party_level_row(20)->hp, 240);
    CHECK_EQ(party_level_row(20)->xp_to_next, 0);

    CHECK(magic_load_spells("assets/spells.txt", reason, sizeof reason));
    CHECK_EQ(magic_spell_count(), 13);
    CHECK(strcmp(magic_spell_at(0)->name, "Mend") == 0);
    CHECK_EQ(magic_spell_at(0)->level, 2);
    CHECK_EQ(magic_spell_at(0)->mp_cost, 3);
    CHECK(strcmp(magic_spell_at(12)->name, "Sunder") == 0);
    CHECK_EQ(magic_spell_at(12)->magnitude, 240);

    /* Multi-word names survive, which is the point of reading the rest
       of the line rather than a single token. */
    int found_frost = 0;
    for (int i = 0; i < magic_spell_count(); i++) {
        if (strcmp(magic_spell_at(i)->name, "Frost Nip") == 0) {
            found_frost = 1;
        }
    }
    CHECK(found_frost);

    /* Effects are real function pointers, not names. */
    for (int i = 0; i < magic_spell_count(); i++) {
        CHECK(magic_spell_at(i)->effect != NULL);
    }
}

static void test_data_tables_reject_bad_files(void)
{
    printf("data tables reject bad files\n");

    char reason[96];

    /* A missing file is a downgrade, not a failure: it reports, and the
       previously loaded table stays usable. */
    CHECK(!party_load_levels("/tmp/no_such_levels.txt", reason, sizeof reason));
    CHECK(reason[0] != '\0');
    CHECK_EQ(party_level_row(1)->hp, 20);      /* still works */

    CHECK(!magic_load_spells("/tmp/no_such_spells.txt", reason, sizeof reason));
    CHECK_EQ(magic_spell_count(), 13);         /* still works */

    static const char *bad_levels[] = {
        "level 1 20 0 5 3 5 8\n",                    /* only one level  */
        "level 0 20 0 5 3 5 8\n",                    /* level 0         */
        "level 99 20 0 5 3 5 8\n",                   /* past the cap    */
        "level 1 -5 0 5 3 5 8\n",                    /* negative hp     */
        "levle 1 20 0 5 3 5 8\n",                    /* typo'd keyword  */
        "level 1 20 0 5 3 5\n",                      /* too few fields  */
        ""                                            /* empty           */
    };
    int n = (int)(sizeof bad_levels / sizeof bad_levels[0]);

    for (int i = 0; i < n; i++) {
        write_text("/tmp/bad_levels.txt", bad_levels[i]);
        reason[0] = '\0';
        CHECK(!party_load_levels("/tmp/bad_levels.txt", reason, sizeof reason));
        CHECK(reason[0] != '\0');
        /* And the good table is still in place. */
        CHECK_EQ(party_level_row(1)->hp, 20);
    }

    /* A duplicate level is caught rather than silently winning. */
    write_text("/tmp/dup_levels.txt",
               "level 1 20 0 5 3 5 8\nlevel 1 99 0 5 3 5 8\n");
    CHECK(!party_load_levels("/tmp/dup_levels.txt", reason, sizeof reason));

    static const char *bad_spells[] = {
        "spell 2 3 35 none teleport Mend\n",    /* effect not compiled in */
        "spell 2 3 35 confused heal Mend\n",    /* unknown status         */
        "spell 2 3 35 none heal\n",             /* no name                */
        "spell 0 3 35 none heal Mend\n",        /* level 0                */
        "spell 5 3 35 none heal Late\nspell 2 3 35 none heal Early\n", /* unsorted */
        "notaspell 2 3 35 none heal Mend\n",
        ""
    };
    int m = (int)(sizeof bad_spells / sizeof bad_spells[0]);

    for (int i = 0; i < m; i++) {
        write_text("/tmp/bad_spells.txt", bad_spells[i]);
        reason[0] = '\0';
        CHECK(!magic_load_spells("/tmp/bad_spells.txt", reason, sizeof reason));
        CHECK(reason[0] != '\0');
        CHECK_EQ(magic_spell_count(), 13);
    }
}

static void test_retuning_without_rebuilding(void)
{
    printf("retuning by editing a file\n");

    char reason[96];

    /* The whole point of Checkpoint D's choice: change the curve without
       touching a line of C. Write a doubled-HP table and check the game
       picks it up. */
    FILE *f = fopen("/tmp/tuned_levels.txt", "w");
    CHECK(f != NULL);
    if (f != NULL) {
        for (int lv = 1; lv <= PARTY_MAX_LEVEL; lv++) {
            fprintf(f, "level %d %d 5 5 3 5 %d\n",
                    lv, 40 * lv, lv == PARTY_MAX_LEVEL ? 0 : 10);
        }
        fclose(f);
    }

    CHECK(party_load_levels("/tmp/tuned_levels.txt", reason, sizeof reason));
    CHECK_EQ(party_level_row(1)->hp, 40);
    CHECK_EQ(party_level_row(5)->hp, 200);

    /* And it really reaches the hero. */
    Player p;
    make_hero(&p, 1, 0);
    CHECK_EQ(p.max_hp, 40);

    /* Put the shipped table back so later tests see the real numbers. */
    CHECK(party_load_levels("assets/levels.txt", reason, sizeof reason));
    CHECK_EQ(party_level_row(1)->hp, 20);
}


static void test_no_integer_overflow(void)
{
    printf("xp and gold cannot overflow\n");

    /* Chapter 22 found both of these with UBSan: p->xp += xp and
       p->gold += reward are undefined behaviour once the total passes
       INT_MAX, not a harmless wrap. */
    Player p;
    make_hero(&p, 1, 0);

    p.xp = PARTY_MAX_XP - 5;
    party_gain_xp(&p, 1000000);
    CHECK(p.xp >= 0);
    CHECK(p.xp <= PARTY_MAX_XP);

    p.xp = 2147483000;              /* absurd, but a save file could say it */
    party_gain_xp(&p, 2000000);
    CHECK(p.xp >= 0);
    CHECK(p.xp <= PARTY_MAX_XP);

    make_hero(&p, 1, PARTY_MAX_GOLD - 5);
    party_add_gold(&p, 1000000);
    CHECK(p.gold >= 0);
    CHECK_EQ(p.gold, PARTY_MAX_GOLD);

    p.gold = 2147483000;
    party_add_gold(&p, 1000000);
    CHECK(p.gold >= 0);

    /* Negative and zero amounts are ignored rather than subtracting. */
    make_hero(&p, 1, 100);
    party_add_gold(&p, 0);
    CHECK_EQ(p.gold, 100);
    party_add_gold(&p, -50);
    CHECK_EQ(p.gold, 100);

    /* Repeated realistic wins never go negative. */
    make_hero(&p, 1, 0);
    for (int i = 0; i < 100000; i++) {
        party_add_gold(&p, 500);
        party_gain_xp(&p, 300);
        CHECK(p.gold >= 0);
        CHECK(p.xp >= 0);
    }
}


static void test_boss_phases(void)
{
    printf("boss phases\n");

    Battle battle;
    battle_begin_boss(&battle, BOSS_BOGWRIGHT);

    const BossType *boss = battle_boss(BOSS_BOGWRIGHT);
    CHECK(boss != NULL);
    CHECK_EQ(battle.enemy_count, 1);
    CHECK_EQ(battle.boss_id, (int)BOSS_BOGWRIGHT);
    CHECK_EQ(battle.enemies[0].hp, boss->type.max_hp);
    CHECK(battle.outcome == BATTLE_ONGOING);

    /* Phase is derived from HP, so walking HP down must walk the phase
       up -- and never back down on its own. */
    int max_hp = boss->type.max_hp;
    int previous = 0;

    for (int hp = max_hp; hp >= 0; hp--) {
        battle.enemies[0].hp = hp;
        int phase = battle_boss_phase(&battle);

        CHECK(phase >= 0 && phase < BOSS_PHASE_COUNT);
        CHECK(phase >= previous);          /* monotonic as HP falls */
        previous = phase;
    }

    /* The documented thresholds, checked at the boundaries. */
    battle.enemies[0].hp = max_hp;
    CHECK_EQ(battle_boss_phase(&battle), 0);

    battle.enemies[0].hp = (max_hp * 2) / 3;
    CHECK_EQ(battle_boss_phase(&battle), 0);

    battle.enemies[0].hp = (max_hp * 2) / 3 - 1;
    CHECK_EQ(battle_boss_phase(&battle), 1);

    battle.enemies[0].hp = max_hp / 3;
    CHECK_EQ(battle_boss_phase(&battle), 1);

    battle.enemies[0].hp = max_hp / 3 - 1;
    CHECK_EQ(battle_boss_phase(&battle), 2);

    battle.enemies[0].hp = 0;
    CHECK_EQ(battle_boss_phase(&battle), 2);

    /* An ordinary encounter has no phases and must not pretend to. */
    Rng rng;
    rng_seed(&rng, 4242u);
    battle_begin(&battle, &rng, (int)AREA_GRUBBIN_VALE, 5);
    CHECK_EQ(battle.boss_id, -1);
    CHECK_EQ(battle_boss_phase(&battle), 0);

    /* A safe area still leaves boss_id saying "not a boss", even though
       it returns early. memset would have left it at 0, which is a real
       BossId -- this is the check that catches that. */
    battle_begin(&battle, &rng, (int)AREA_CASTLE_HOLLIS, 5);
    CHECK_EQ(battle.boss_id, -1);

    /* Every boss must exist, be beatable, and pay out a key item. */
    for (int id = 0; id < BOSS_COUNT; id++) {
        const BossType *b = battle_boss((BossId)id);
        CHECK(b != NULL);
        CHECK(b->type.name != NULL);
        CHECK(b->type.max_hp > 0);
        CHECK(b->reward_item >= 0 && b->reward_item < ITEM_TYPE_COUNT);
        CHECK(item_type((ItemId)b->reward_item)->key_item == 1);

        for (int phase = 0; phase < BOSS_PHASE_COUNT; phase++) {
            CHECK(b->phase_lines[phase] != NULL);
        }
    }

    CHECK(battle_boss((BossId)-1) == NULL);
    CHECK(battle_boss((BossId)BOSS_COUNT) == NULL);
}

static void test_boss_fight_terminates(void)
{
    printf("boss fights end\n");

    /* The same guarantee the ordinary battle test makes: whoever wins,
       the fight must stop. A boss that gets stronger in its last phase
       is exactly the shape of thing that could loop forever. */
    for (int id = 0; id < BOSS_COUNT; id++) {
        for (unsigned seed = 1; seed <= 60; seed++) {
            Rng rng;
            rng_seed(&rng, seed);

            Player hero;
            make_hero(&hero, PARTY_MAX_LEVEL, 0);
            hero.equipped[SLOT_WEAPON] = gear_count(SLOT_WEAPON) - 1;
            hero.equipped[SLOT_ARMOUR] = gear_count(SLOT_ARMOUR) - 1;

            Battle battle;
            battle_begin_boss(&battle, (BossId)id);

            int rounds = 0;
            while (battle.outcome == BATTLE_ONGOING && rounds < 4000) {
                battle_round(&battle, &hero, &rng, 0, 100);
                rounds++;
            }

            CHECK(battle.outcome != BATTLE_ONGOING);
            CHECK(rounds < 4000);
            CHECK(hero.hp >= 0);
        }
    }
}

static void test_world_gating(void)
{
    printf("world gating and boss triggers\n");

    World *world = world_create();
    CHECK(world != NULL);
    if (world == NULL) {
        return;
    }

    /* Every area's map must have loaded, or the game cannot be played
       to the end. This is the check that catches a mistyped path. */
    for (int i = 0; i < AREA_COUNT; i++) {
        CHECK(world->maps[i] != NULL);
        CHECK(world->areas[i].name != NULL);
    }

    /* Every warp must land somewhere walkable, in a real area. A typo
       here strands the player in a wall. */
    for (int i = 0; i < AREA_COUNT; i++) {
        const Area *area = &world->areas[i];

        for (int w = 0; w < area->warp_count; w++) {
            const Warp *warp = &area->warps[w];

            CHECK(warp->destination >= 0 && warp->destination < AREA_COUNT);
            CHECK(map_is_walkable(world->maps[warp->destination],
                                  warp->destination_x, warp->destination_y));

            /* A gated warp must say why it is shut. */
            if (warp->requires_item >= 0) {
                CHECK(warp->requires_item < ITEM_TYPE_COUNT);
                CHECK(warp->locked_speech != NULL);
            }
        }

        /* A boss must stand somewhere the player can actually reach. */
        if (area->boss.boss_id >= 0) {
            CHECK(area->boss.boss_id < BOSS_COUNT);
            CHECK(map_is_walkable(world->maps[i],
                                  area->boss.x, area->boss.y));
        }
    }

    /* The two gates that enforce the story's order. */
    world->current = AREA_CASTLE_HOLLIS;

    const Warp *to_mudwick = world_warp_at(world, 7, 15);
    CHECK(to_mudwick != NULL);
    CHECK_EQ(to_mudwick->requires_item, (int)ITEM_FRAGMENT_I);

    const Warp *to_court = world_warp_at(world, 28, 15);
    CHECK(to_court != NULL);
    CHECK_EQ(to_court->requires_item, (int)ITEM_FRAGMENT_II);

    /* Boss triggers sit where the tables say they do. */
    world->current = AREA_WETWOOD_BARROW;
    const BossTrigger *bog = world_boss_at(world, 20, 9);
    CHECK(bog != NULL);
    CHECK_EQ(bog->boss_id, (int)BOSS_BOGWRIGHT);
    CHECK(world_boss_at(world, 1, 1) == NULL);

    world->current = AREA_THE_CROWNLESS_COURT;
    const BossTrigger *vex = world_boss_at(world, 10, 1);
    CHECK(vex != NULL);
    CHECK_EQ(vex->boss_id, (int)BOSS_VEX);

    /* A safe area has no trigger anywhere on its map. */
    world->current = AREA_MUDWICK;
    for (int y = 0; y < world->maps[AREA_MUDWICK]->height; y++) {
        for (int x = 0; x < world->maps[AREA_MUDWICK]->width; x++) {
            CHECK(world_boss_at(world, x, y) == NULL);
        }
    }

    world_destroy(world);
}

int main(void)
{
    test_base_damage();
    test_critical_damage();
    test_variance();
    test_variance_vanishes_on_small_numbers();
    test_hit_chance();
    test_flee_chance();
    test_rng();
    test_range_covers_every_value();
    test_rolled_damage_stays_sane();
    test_level_table();
    test_xp_and_levelling();
    test_battle_setup();
    test_group_size_scales_with_level();
    test_battle_terminates();
    test_fleeing_terminates();
    test_inventory_growth();
    test_inventory_removal();
    test_inventory_many_items();
    test_config();
    test_dialog_paging_loses_nothing();
    test_spell_table();
    test_spell_casting();
    test_status_flags();
    test_equipment();
    test_enemy_only_turn();
    test_shop_lookup();
    test_shop_buying();
    test_inn();
    test_gold_never_negative();
    test_save_round_trip();
    test_save_refuses_bad_files();
    test_load_never_partially_applies();
    test_data_tables_load();
    test_data_tables_reject_bad_files();
    test_retuning_without_rebuilding();
    test_no_integer_overflow();
    test_boss_phases();
    test_boss_fight_terminates();
    test_world_gating();

    return test_report();
}
