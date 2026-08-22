#include "test.h"
#include "combat_math.h"
#include "rng.h"

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

    return test_report();
}
