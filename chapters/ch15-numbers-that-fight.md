# Chapter 15: Numbers That Fight

## Where we are

You have a world, a cast, and clean modes. What you don't have is any
reason to fear the roads. Before you can build a battle — which is
Chapter 16 — you need the arithmetic underneath it: how much a hit hurts,
whether it lands at all, and where the randomness comes from. That's this
chapter, and it comes with the first tests in this course.

At Checkpoint C you chose AGI-sorted turn order and an FF1-style damage
formula with variance and criticals. Everything below implements exactly
that.

## The problem

Combat maths is where games get quietly, permanently broken. A formula
that produces negative damage, or zero damage forever, or a "random"
number that's the same every run, will not crash — it will just make your
game bad in a way that's hard to trace back to a single line. And unlike
a segfault, no sanitizer will tell you.

The fix is to make the arithmetic **pure and testable**, and to write the
tests now, while the formula is six lines and you still remember what it's
supposed to do.

## C concept spotlight

### Pure functions

A **pure function** depends only on its arguments and changes nothing
outside itself. Same inputs, same output, every time, forever.

```c
int combat_base_damage(int attack, int weapon_power, int defense)
{
    int offence = attack + weapon_power;
    int base = offence - (defense / 2);

    if (base < 1) {
        base = 1;
    }

    return base;
}
```

Nothing here reads a global, calls a random number generator, prints
anything, or touches a `Player`. That's what makes it trivially testable:
you can assert `combat_base_damage(10, 0, 6) == 7` and that assertion is
true forever, on any machine, in any order, with no setup.

Compare that to a function taking `const Player *attacker` and rolling
randomness internally — to test it you'd have to build a `Player`, control
the RNG, and hope nothing else moved. **Pushing randomness and state to
the edges, keeping the core pure, is the single most useful design habit
in this chapter**, and it's why `combat_math.c` has no idea the rest of
the game exists.

### Randomness you control: `xorshift32`

C's standard `rand()` is available and this course does not use it, for
reasons `CONTENT.md` §9.4 spelled out and which are worth understanding:

- Its quality is **implementation-defined** — the same seed gives
  different sequences on different platforms, so a bug someone reports
  can't be reproduced from a seed.
- It has one hidden global state, so any code anywhere calling `rand()`
  shifts every other caller's sequence.
- You can't save its state, so you can't replay a run.

A whole replacement is six lines:

```c
typedef struct {
    uint32_t state;
} Rng;

uint32_t rng_next(Rng *rng)
{
    uint32_t x = rng->state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    rng->state = x;
    return x;
}
```

This is **xorshift32**: three shift-and-XOR steps that scramble a 32-bit
number into another one. `^` is bitwise XOR, `<<` and `>>` shift bits left
and right. You don't need to understand *why* those three specific
constants work (that's number theory) — you need to know that they do,
that the state is one `uint32_t` you can save, and that the same seed
always replays the same sequence.

`uint32_t` comes from `<stdint.h>` and means "exactly 32 bits, unsigned."
Being exact matters here: the algorithm's behaviour depends on bits
falling off the end at 32, which `unsigned int` doesn't guarantee.

**There is one forbidden value.** Zero XOR-shifted is zero, forever — a
generator stuck producing nothing:

```c
void rng_seed(Rng *rng, uint32_t seed)
{
    rng->state = (seed == 0) ? 0x9E3779B9u : seed;
}
```

That silent substitution is worth a comment in real code, because a
"random number generator that always returns 0" is a memorably confusing
bug to debug at 1am.

### Integer overflow is undefined behaviour

Damage formulas add and multiply integers, so it's time to be precise
about what happens when those get too big.

```c
int attack = INT_MAX - 5;
int weapon = 100;
int sum = attack + weapon;
```

```
$ ./ovf
INT_MAX = 2147483647
(INT_MAX - 5) + 100 = -2147483554
```

The number wrapped around to a large negative. And critically — **signed
integer overflow is undefined behaviour**, not defined wrapping. The
compiler is entitled to assume it never happens and optimize accordingly,
which means the result you see at `-O0` may differ from `-O2`. UBSan
catches it:

```
$ gcc -std=c17 -Wall -Wextra -g -fsanitize=undefined -o ovf_ub ovf.c
$ ./ovf_ub
ovf.c:10:9: runtime error: signed integer overflow: 2147483642 + 100 cannot be represented in type 'int'
```

*Unsigned* overflow is different — it's fully defined to wrap:

```
UINT32_MAX + 1 = 0  (defined: wraps to 0)
```

That's exactly why `rng_next` works on `uint32_t`: the wrapping is the
algorithm, and on unsigned types it's guaranteed rather than undefined.

For this game's stat ranges (attack in the tens, HP in the hundreds)
overflow is not a practical risk. Knowing where the cliff is still matters
— and Chapter 22 comes back to hunt for cases like it.

### Order of operations in integer arithmetic

Here is a bug that looks like a style preference:

```c
int delta = (base * offset) / 100;    /* correct */
int delta = base * (offset / 100);    /* always zero */
```

With `base = 100` and `offset = 12`, the first gives `12`; the second
computes `12 / 100`, which integer division truncates to `0`, and then
multiplies by `100` to get `0`. **Multiply before you divide** whenever
you're scaling by a percentage in integers. This is Chapter 2's
integer-division trap, resurfacing in a place where it silently disables
a whole game system rather than printing a wrong percentage.

## Building a test harness

You do not need a framework. Twenty lines is enough, in `test.h`:

```c
static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        tests_run++;                                                    \
        if (!(expr)) {                                                  \
            tests_failed++;                                             \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);    \
        }                                                               \
    } while (0)

#define CHECK_EQ(actual, expected)                                      \
    do {                                                                \
        tests_run++;                                                    \
        int a_ = (actual);                                              \
        int e_ = (expected);                                            \
        if (a_ != e_) {                                                 \
            tests_failed++;                                             \
            printf("  FAIL %s:%d: %s == %d, expected %d\n",             \
                   __FILE__, __LINE__, #actual, a_, e_);                \
        }                                                               \
    } while (0)

static int test_report(void)
{
    printf("\n%d checks, %d failed\n", tests_run, tests_failed);
    return tests_failed != 0;
}
```

Four pieces of macro technique are doing real work here:

- **`__FILE__` and `__LINE__`** are predefined macros the preprocessor
  replaces with the current file name and line number — that's how a
  failure can tell you where it happened.
- **`#expr`** is the *stringizing* operator: it turns the macro argument
  into a string literal, so `CHECK(x > 3)` can print `"x > 3"`. This is
  something only a macro can do; a function receives the value `x > 3`
  evaluated to `1` or `0`, and can never recover the text.
- **`do { ... } while (0)`** wraps the body so the macro behaves like a
  single statement. Without it, `if (cond) CHECK(x); else ...` would break
  in confusing ways, because the macro's braces would end the `if`
  prematurely. This idiom looks pointless and is not.
- **`a_` and `e_` evaluate the arguments exactly once.** Writing
  `(actual) != (expected)` directly in the comparison *and* in the printf
  would evaluate them twice — and `CHECK_EQ(rng_next(&r), 5)` would then
  draw two different random numbers and compare the wrong ones. The
  trailing underscore is a small guard against colliding with a variable
  named `a` at the call site.

Returning `tests_failed != 0` from `main` makes the exit status non-zero
on failure, which is what lets `make test` actually fail a build rather
than printing sadly and succeeding.


### Property tests beat golden numbers alone

A single expected damage value can rot when the formula changes on
purpose. A **monotonicity** check ("more defence never increases damage")
survives retuning and still catches sign errors. Practice `03_monotonic`
is that idea without the game binary; keep the same test in the chapter
harness.


## Apply it

Create `rng.h`/`rng.c` and `combat_math.h`/`combat_math.c` as shown above,
plus `rng_range` for bounded values:

```c
int rng_range(Rng *rng, int low, int high)
{
    if (high < low) {
        return low;
    }

    uint32_t span = (uint32_t)high - (uint32_t)low + 1u;
    return low + (int)(rng_next(rng) % span);
}
```

The `uint32_t` cast is deliberate: `high - low` as `int` could overflow
for extreme values, while the unsigned subtraction is defined. And yes,
`%` introduces a slight bias toward low values (2³² isn't evenly divisible
by most spans) — for a range of a few dozen out of four billion, the bias
is far below anything a player could perceive. Knowing that a compromise
exists, and that you chose it deliberately, is different from not knowing.

The rest of `combat_math.c` implements Checkpoint C's formula, with the
pure parts separated from the rolling parts:

```c
int combat_apply_variance(int base, int roll)
{
    /* roll 0 -> -12%, roll 12 -> 0%, roll 24 -> +12%. */
    int offset = roll - COMBAT_VARIANCE_PERCENT;

    /* Multiply before dividing so the percentage does not vanish to
       zero under integer division for small base values. */
    int delta = (base * offset) / 100;

    int result = base + delta;
    if (result < 1) {
        result = 1;
    }

    return result;
}
```

Notice `combat_apply_variance` takes a `roll` rather than an `Rng *`. That
one decision is what lets the test suite check **every legal roll against
hundreds of base values exhaustively** — no randomness, no sampling, no
flakiness. The RNG only appears in `combat_roll_damage`, which composes
the pure pieces:

```c
int combat_roll_damage(Rng *rng, int attack, int weapon_power, int defense,
                       int *out_was_critical)
{
    int critical = (rng_range(rng, 1, COMBAT_CRIT_IN_N) == 1);

    if (out_was_critical != NULL) {
        *out_was_critical = critical;
    }

    int base = critical
             ? combat_critical_damage(attack, weapon_power)
             : combat_base_damage(attack, weapon_power, defense);

    int roll = rng_range(rng, 0, 2 * COMBAT_VARIANCE_PERCENT);

    return combat_apply_variance(base, roll);
}
```

`out_was_critical` is an **out-parameter** — a pointer the function writes
a second result through, since C returns only one value. The `NULL` check
makes it optional, so a caller that doesn't care can pass `NULL`. Both are
Chapter 7 techniques doing ordinary work.

### The tests

`test_combat.c` covers each function's ordinary case, its boundaries, and
the properties that must hold everywhere. A representative slice:

```c
static void test_base_damage(void)
{
    printf("base damage\n");

    /* An even fight: 10 attack, no weapon, 6 defence -> 10 - 3 = 7. */
    CHECK_EQ(combat_base_damage(10, 0, 6), 7);

    /* A weapon adds its power before defence is subtracted. */
    CHECK_EQ(combat_base_damage(10, 4, 6), 11);

    /* Overwhelming defence still lets 1 through, never 0 or negative. */
    CHECK_EQ(combat_base_damage(5, 0, 40), 1);
    CHECK_EQ(combat_base_damage(1, 0, 999), 1);
}
```

And the exhaustive property test, which is where pure functions really pay
off:

```c
for (int base = 1; base <= 500; base++) {
    for (int roll = 0; roll <= 2 * COMBAT_VARIANCE_PERCENT; roll++) {
        int result = combat_apply_variance(base, roll);

        CHECK(result >= 1);
        CHECK(result <= base + (base * COMBAT_VARIANCE_PERCENT) / 100);
    }
}
```

That's 12,500 combinations checked in microseconds. There is no
statistical hand-waving and no possibility of a lucky pass.

The RNG gets tested on the properties that matter for a *game*, not on
statistical quality:

```c
/* The same seed must give the same sequence -- this is the whole
   reason for carrying our own generator. */
rng_seed(&a, 12345);
rng_seed(&b, 12345);
for (int i = 0; i < 100; i++) {
    CHECK(rng_next(&a) == rng_next(&b));
}

/* Seed 0 is the forbidden state and must be replaced, not accepted. */
rng_seed(&a, 0);
CHECK(a.state != 0);
CHECK(rng_next(&a) != 0);
```

Add a `test` target to the `Makefile`. Note what it links:

```make
# The test runner links only the pure modules -- no ncurses, no game
# loop. That is a direct benefit of keeping the maths free of the world.
TEST_SOURCES = test_combat.c combat_math.c rng.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_TARGET = run_tests

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_OBJECTS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)
```

Three files, no ncurses, no terminal. If `combat_math.c` had reached into
the game state, the test build would have had to drag in the whole
program.

### Seeing it in the game

So the chapter ships something visible, add a "Practice swing" item to the
Chapter 14 menu that attacks a straw dummy, plus an `agility` field on
`Player` (set to `5`) and an `Rng` on `Game`, seeded at startup:

```c
rng_seed(&game->rng, (uint32_t)time(NULL));
```

```c
int was_critical = 0;
int hit = combat_roll_hit(&game->rng, game->player.agility, 5);
char line[DIALOG_LINE_LEN];

if (hit) {
    int dmg = combat_roll_damage(&game->rng, game->player.attack, 0, 4,
                                 &was_critical);
    snprintf(line, sizeof line, "%s the dummy for %d.",
             was_critical ? "CRITICAL" : "You hit", dmg);
} else {
    snprintf(line, sizeof line, "You swing and miss.");
}

dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
game->mode = MODE_DIALOG;
```

Seeding from `time(NULL)` means a different sequence each run.
`CONTENT.md` §9.4 suggests storing the seed in save files so a run can be
replayed — that's Chapter 20's business, but the design is ready for it
because the entire generator state is one number.

## Compile and run

```bash
make clean
make test
```

```
base damage
critical damage
variance
variance on small numbers (documented limitation)
hit chance
flee chance
rng
rng_range coverage
rolled damage

87839 checks, 0 failed
```

Then the game:

```bash
make
./game
```

Open the menu with `m`, pick "Practice swing", and confirm a few times:

```
Grubbin Vale
   Status
   Commission
 > Practice swing
   Close menu
```

```
 You hit the dummy
 for 4.
```

```
 CRITICAL the dummy
 for 12.
```

```
 You swing and miss.
```

And the sanitizer build, as always from Chapter 10 onward:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

## What just happened — and a real finding

Swing the practice dummy repeatedly at level 1 and you'll notice something:
every non-critical hit does **exactly 4**. Never 3, never 5. The variance
you just implemented appears to do nothing.

It isn't broken. Work it through: base damage is
`(6 + 0) - (4 / 2)` = `4`. Twelve percent of 4 is 0.48. Integer division
throws the fraction away, so `delta` is `0` for every roll in the band.

Checked across the whole range:

```
  base  1 -> range [1, 1]   <-- variance invisible
  base  4 -> range [4, 4]   <-- variance invisible
  base  8 -> range [8, 8]   <-- variance invisible
  base  9 -> range [8, 10]
  base 12 -> range [11, 13]
  base 20 -> range [18, 22]
  base 45 -> range [40, 50]
  base 150 -> range [132, 168]
```

**Below base 9, ±12% integer variance rounds entirely away.** Early-game
combat is fully deterministic; variance only becomes visible as your
numbers grow. That's a genuine consequence of the formula meeting integer
arithmetic, and it's exactly the kind of thing `CONTENT.md` meant when it
said the stat table "will need playtesting" rather than presenting the
numbers as tuned.

You have three honest options, and this is a design decision, not a bug
fix: accept it (early fights being predictable is arguably fine, and it's
what the FF1-derived formula does), raise the base numbers so variance
bites sooner, or switch to fixed-point arithmetic to keep fractions. This
course accepts it — and **pins it down with a test** so nobody
"corrects" it later without realising it was deliberate:

```c
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
```

A test that documents *why* behaviour is the way it is, is worth as much
as one that catches a regression.

## Do the tests actually work?

A test suite that passes is not evidence of anything until you've seen it
fail. Break the code on purpose and check:

**Remove the damage floor** (`if (base < 1) base = 1;`):

```
  FAIL test_combat.c:19: combat_base_damage(5, 0, 40) == -15, expected 1
  FAIL test_combat.c:20: combat_base_damage(1, 0, 999) == -498, expected 1
  FAIL test_combat.c:23: combat_base_damage(0, 0, 0) == 0, expected 1
87637 checks, 3 failed
```

**Clamp hit chance to 100 instead of 99:**

```
  FAIL test_combat.c:80: combat_hit_chance(200, 0) == 100, expected 99
87637 checks, 1 failed
```

**Divide before multiplying in the variance** (`base * (offset / 100)`):

```
  FAIL test_combat.c:46: combat_apply_variance(100, 0) == 100, expected 88
  FAIL test_combat.c:47: combat_apply_variance(100, 24) == 100, expected 112
87637 checks, 2 failed
```

All three caught, with the exact line and the actual value. Deliberately
breaking your own code to confirm the tests notice is called **mutation
testing**, and doing it once by hand after writing a suite is one of the
highest-value ten minutes in software.

## Common errors

**Forgetting `<stddef.h>` for `NULL` in a module that includes no other
headers:**

```
combat_math.c: In function ‘combat_roll_damage’:
combat_math.c:71:29: error: ‘NULL’ undeclared (first use in this function)
   71 |     if (out_was_critical != NULL) {
      |                             ^~~~
combat_math.c:2:1: note: ‘NULL’ is defined in header ‘<stddef.h>’; this is probably fixable by adding ‘#include <stddef.h>’
```

This one happened while writing this chapter. `NULL` feels built into the
language but is a macro from a header — usually dragged in accidentally by
`<stdio.h>` or `<stdlib.h>`. A file pure enough to need neither has to ask
for it explicitly, which is a small sign you've succeeded at keeping the
module clean.

**A macro without `do { } while (0)`:**

```c
#define CHECK(expr) tests_run++; if (!(expr)) tests_failed++;

if (x) CHECK(y);
else   printf("no\n");
```

```
badmacro.c: In function ‘main’:
badmacro.c:3:21: warning: macro expands to multiple statements [-Wmultistatement-macros]
    3 | #define CHECK(expr) tests_run++; if (!(expr)) tests_failed++;
      |                     ^~~~~~~~~
badmacro.c:6:12: note: in expansion of macro ‘CHECK’
```

The macro expands to two statements, so `if (x)` swallows only the first
and the rest runs unconditionally. Modern GCC catches this specific shape
with `-Wmultistatement-macros` (part of `-Wall`), which is a genuine
kindness — older compilers gave you a baffling `‘else’ without a previous
‘if’` several lines away, or worse, silently wrong control flow. The
`do { } while (0)` wrapper prevents the whole class of problem, so use it
in every multi-statement macro rather than relying on the warning.

**Seeding the RNG inside a loop:**

```c
for (int i = 0; i < 10; i++) {
    Rng rng;
    rng_seed(&rng, 42);
    printf("%d ", rng_range(&rng, 1, 100));
}
```

Prints the same number ten times. Reseeding restarts the sequence — seed
once, at startup, and carry the `Rng` around. This is a mistake that looks
like a broken generator and is really a misplaced call.

## Exercises

> **Practice drills:** `code/ch15/practice/` (`01_clamp_damage`–
> `04_xorshift_seed`; `make && make check`) before exercise 2. Pure math lives
> in drills; the game only *wires* it. Do **not** delete the production damage
> floor or rewrite flee maths inside `game.c` to "try something else."

1. *Practice.* Complete the practice folder (damage floor, flee chance,
   monotonicity, seeded RNG sequences).
2. *Durable property.* Add the defence-monotonicity test to the real
   combat test harness (same shape as practice `03`). Does current
   `combat_base_damage` pass?
3. *Wiring, not rewrite.* Expose flee chance somehow lasting (menu "Test
   your nerve" is fine) — call existing `combat_flee_chance`, do not
   reimplement in `game.c`.
4. *Open-ended:* Sketch `combat_spell_damage` (ignore physical defence,
   reuse variance). What properties would you test?

<details>
<summary>Solutions</summary>

1. See `code/ch15/practice/solutions/`.
2. Loop defence 0..200; `CHECK(current <= previous)`. Should pass.
3. Menu item + `combat_flee_chance` + `rng_range` + dialog line.
4. `combat_spell_damage(rng, power, magic_stat)`; floor at 1; variance
   reused; defence not a parameter.

</details>

## Next up

You have damage you can trust and a generator you can replay. Chapter 16
spends both: enemies with stats, AGI-sorted turn order, a `MODE_BATTLE`
that slots into the Chapter 14 dispatch table, random encounters on a step
counter — with the off switch you asked for at Checkpoint C — and, at
last, experience points and levels.
