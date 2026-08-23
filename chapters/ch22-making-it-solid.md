# Chapter 22: Making It Solid

## Where we are

You have written six parsers, seven dispatch-table modes, three dynamic
allocations and about 4,400 lines of C. Every snapshot has compiled without
warnings and run clean under the sanitizers.

And you have been getting away with things.

This chapter finds out which ones. It is the chapter where a working game
gets put under tools that don't care whether it works, and asked whether it
is *correct* — which is a different question, and one you can't answer by
playing.

## The problem

Twenty-one chapters of "it ran fine" is not evidence. Chapter 10 made that
point with a use-after-free that printed a plausible number and exited
zero. The bugs that survive to this point are exactly the ones that don't
announce themselves: undefined behaviour that happens to work, memory read
before it was written, arithmetic that only breaks at values you never
tested.

## C concept spotlight

### Two sanitizers, one blind spot

You have used AddressSanitizer since Chapter 10, and it has been genuinely
excellent — out-of-bounds reads and writes, use-after-free, double-free,
leaks. But Chapter 17 established something important with a live
demonstration:

> **ASan does not detect reads of uninitialised memory.**

That run printed `0xBEBEBEBE` garbage and reported nothing, because the
memory was legitimately owned — just never written. ASan tracks *where*
memory is, not *whether it has a value*.

That class of bug needs a different tool. **Valgrind's memcheck** simulates
your program instruction by instruction, tracking the definedness of every
bit. It is dramatically slower — 20 to 50 times — and it sees things ASan
structurally cannot.

The two are complements, not alternatives:

| | AddressSanitizer + UBSan | Valgrind memcheck |
|---|---|---|
| Out-of-bounds | yes, fast | yes, slower |
| Use-after-free | yes | yes |
| Leaks | yes | yes, with more detail |
| **Uninitialised reads** | **no** | **yes** |
| Signed overflow, bad shifts | yes (UBSan) | no |
| Speed | ~2× slower | ~30× slower |
| Needs recompiling | yes | no |

Run both. This chapter's `Makefile` gets a target for each:

```make
# The test suite under memcheck. This is the one that catches
# uninitialised reads, which the sanitizers do not.
valgrind: $(TEST_TARGET)
	valgrind --leak-check=full --track-origins=yes \
	         --suppressions=valgrind.supp --error-exitcode=9 ./$(TEST_TARGET)
```

`--track-origins=yes` is what makes an uninitialised-value report
actionable: without it Valgrind tells you *where the value was used*, which
is rarely where the bug is. With it, you also get where the value came
from.

> **If Valgrind refuses to start**, with a wall of text about "a function
> redirection which is mandatory for this platform-tool combination," it is
> missing debug symbols for the dynamic linker. The fix on Arch is one
> environment variable, and the suggested `libc6-dbg` / `glibc-debuginfo`
> packages in its error message do not exist here. See
> [Appendix A](../appendices/a-wsl-troubleshooting.md) — *Valgrind: "Fatal
> error at startup"*. Sort this before reading on; the rest of the chapter
> assumes Valgrind runs.

### Assertions: stating what you already assume

An **assertion** is a claim that must be true, checked at runtime, that
crashes loudly if it isn't:

```c
#include <assert.h>

assert(battle->target >= 0 && battle->target < battle->enemy_count);
```

Assertions are not error handling. Error handling is for things that *can*
happen — a missing file, a failed `malloc`, a player typing nonsense.
Assertions are for things that **cannot happen unless the code is wrong**.
A save file with a bad level is an error to report; a target index outside
the enemy array is a bug to crash on.

The distinction matters because `assert` compiles to nothing when `NDEBUG`
is defined:

```bash
gcc -DNDEBUG ...    # every assert() disappears
```

Which means: **never put anything with a side effect inside an assert.**

```c
assert(inventory_add(inv, id, 1));   /* WRONG: the item vanishes in release */
```

Good places for assertions are exactly where a comment would otherwise say
"this is always true":

```c
/* Growth must have produced room, or the write below is out of
   bounds. This is the invariant inventory_grow exists to maintain. */
assert(inv->count < inv->capacity);
assert(inv->stacks != NULL);

inv->stacks[inv->count].id = id;
```

That assertion documents `inventory_grow`'s contract *and* enforces it. If
a future change breaks the relationship, the program stops there, rather
than corrupting the heap and failing somewhere unrelated.

### Turning up the compiler

Before running anything, ask the compiler for more. Beyond the standard
flags this course has used:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
    -Wsign-conversion -Wcast-qual -Wnull-dereference -Wformat=2 \
    -Wswitch-enum -Wstrict-prototypes -Wmissing-prototypes -c *.c
```

Run on this project, that produces twenty-two warnings in exactly two
families — a mild result, which is the point of having kept the build
warning-free all along:

```
     20 [-Wswitch-enum]
      2 [-Wsign-conversion]
```

Neither family is a bug.

`-Wswitch-enum` is stricter than the `-Wswitch` you already have: it wants
every enum value listed explicitly even when a `default:` arm exists. All
twenty are input and item-effect handlers that deliberately end in
`default:` precisely so new enum values get a safe fallback:

```
game.c:539:5: warning: enumeration value ‘INPUT_FLEE’ not handled in
switch [-Wswitch-enum]
```

There is a real argument on the other side — listing every value means the
compiler tells you about every `switch` that needs attention when you add a
mode — but it is a design change, not a bug fix, and this late in a project
it is not worth the churn.

The two sign-conversion hits are both the same line of the `CHECK_EQ` test
macro:

```
test.h:25:18: warning: conversion to ‘int’ from ‘unsigned int’ may change
the sign of the result [-Wsign-conversion]
   25 |         int a_ = (actual);
```

That is the macro being handed an `unsigned` status flag by two of its
callers. Inside a test harness, comparing a small flag as `int`, it is
harmless.

Worth knowing: **a warning you have decided to ignore should be a decision,
not a habit.** These two families are decisions — and note that they live
in the *test* code and in deliberate `default:` arms, not in the game's
logic.

## The hunt

### Fuzzing the parsers

Five parsers now read files: `map_load`, `config_load`, `save_read`,
`party_load_levels` and `magic_load_spells`. (The save-game inventory makes
a sixth format, but it is read inside `save_read` rather than through an
entry point of its own.) Every one of them takes input a player can edit in
a text editor, which makes each an untrusted-input boundary.

**Fuzzing** — a new word: feeding a program randomly generated input to see
whether it survives — is the cheapest way to test that boundary. Create
`fuzz_parsers.c` alongside `test_combat.c`. The heart of it is the
character generator:

```c
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
```

Purely random bytes would almost never produce something a parser gets deep
into — it would reject line 1 and stop, over and over. Biasing toward `=`,
digits, spaces and `#` means far more inputs reach the interesting code
paths. The full file is in `code/ch22/fuzz_parsers.c`; it writes 4,000
random files and pushes each one through all five parsers.

The `Makefile` gets two targets for it — one plain, one under the
sanitizers, which is where a parser bug actually shows up:

```make
fuzz: $(FUZZ_TARGET)
	./$(FUZZ_TARGET)

fuzz-sanitize:
	$(MAKE) clean
	$(MAKE) $(FUZZ_TARGET) \
	        CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined"
	./$(FUZZ_TARGET)
```

```bash
make fuzz-sanitize 2>/dev/null
```

The rejection messages go to `stderr`, so redirecting it leaves just the
verdict (drop the `2>/dev/null` if you want to watch four thousand files
being turned away):

```
fuzzed 4000 random files through 5 parsers: no crash
level 1 hp still 20 (expect 20)
spell count still 13 (expect 13)
```

The last two lines matter as much as the first. Not crashing is the low
bar; the parsers also had to leave the good tables intact after 4,000
rejections, which is the "no partial load" rule from Chapters 20 and 21
holding under pressure. If a malformed file had been half-adopted, level
1's HP or the spell count would have moved, and the fuzzer exits non-zero.

### Monkey-testing the game

Then the game itself: 1,500 random keypresses through a live ncurses
session, built with ASan and UBSan. The obvious way to do that is to pipe
random keys in:

```bash
printf 'Wick\n\nwasdwasdq' | ./game        # does NOT work
```

That hangs, spinning at full speed and producing megabytes of redrawn
frames. It is worth understanding why, because the reason is pure C.

`entity_create_player` reads the hero's name with `fgets`, and `fgets` is
buffered: it doesn't read five bytes from the pipe, it reads *as much as
the pipe will give it* into a `FILE` buffer and hands you the first line.
For a short pipe, that swallows the entire input. Afterwards `getch()` —
which reads from file descriptor 0 directly, not through `stdio` — finds
nothing left and returns `ERR` forever. Two input mechanisms, one file
descriptor, no coordination.

There is a second reason: ncurses puts the terminal in `cbreak` mode to get
keys without Enter, and a pipe is not a terminal. To drive the game the way
a player does, the input has to come from a **pseudo-terminal** — a kernel
object that behaves like a terminal on one end and a pipe on the other.

A short Python script is the easiest way to get one (this is a testing
tool, not part of the game):

```python
import os, pty, random, select, subprocess

random.seed(22)
script = ("Wick\n\n"
          + "".join(random.choice("wasdmf \n") for _ in range(1500))
          + "q"          # quit the game loop
          + "h\n").encode()   # answer the Chapter 3 duel on the way out

master, slave = pty.openpty()
p = subprocess.Popen(["./game"], stdin=slave, stdout=slave, stderr=slave)
os.close(slave)

sent = 0
while p.poll() is None:
    r, w, _ = select.select([master], [master] if sent < len(script) else [], [], 0.1)
    if r and not os.read(master, 65536):
        break
    if w:
        sent += os.write(master, script[sent:sent + 256])
```

Note the single `q`, not forty. Sending a pile of them looks harmless and
isn't: the first one ends the game loop, and `duel_run`'s input drain eats
every character after it up to the next newline — including the `h` the
duel is waiting for. The program then sits in `ask_call` forever. Small
input-handling details like that are exactly what a monkey test is for.

Draining the master end matters too. If nobody reads the output, the pty
buffer fills, the game blocks on `refresh()`, and the whole thing deadlocks
looking exactly like a hang in your code.

```
1500 random keypresses through a live game
diagnostics: 0
exit: 0
```

This is cruder than it sounds, and it exercises transitions no human would
try — opening the menu mid-battle, confirming an empty inventory,
cycling gear while a text box is up. Those mode transitions are precisely
where Chapters 18 and 19 had bugs.

The same script drives the Valgrind runs later in this chapter. That
matters more than it looks: a run killed by `timeout` still prints a leak
summary, but it is the leak summary of a program that never reached
`game_destroy`. Only a **clean exit** tells you the teardown path is
correct.

### Bug 1: integer overflow, found by UBSan

Testing extreme values found a real one:

```
party.c:185:11: runtime error: signed integer overflow: 100 + 2147483637 cannot be represented in type 'int'
```

The code was the most ordinary line imaginable:

```c
p->xp += xp;
```

Chapter 15 explained that signed overflow is undefined behaviour, not
wrapping. Here it was producing **negative XP and negative gold**:

```
  !! xp went negative after overflow: -2147483559
  !! gold overflowed to -2147483554
```

Reachable? Honestly, not by playing — you would need millions of battles.
But it is reachable from a *save file*, which Chapter 20 invites the player
to edit, and "undefined behaviour that needs an absurd input" is still
undefined behaviour. The fix is to clamp, in exactly one place each:

```c
/* Clamp rather than add blindly: p->xp + xp can overflow, and signed
   overflow is undefined behaviour, not a wrap to a negative. */
if (xp > PARTY_MAX_XP - p->xp) {
    p->xp = PARTY_MAX_XP;
} else {
    p->xp += xp;
}
```

Note the form of the check. The obvious version is wrong:

```c
if (p->xp + xp > PARTY_MAX_XP)      /* the overflow happens IN the check */
```

You must test *before* performing the addition, by rearranging it:
`xp > LIMIT - p->xp`. Subtracting can't overflow here because both operands
are non-negative and `p->xp <= LIMIT`.

Gold got the same treatment as a function, so the ceiling lives in one
place:

```c
void party_add_gold(Player *p, int amount);
```

and every `player.gold += reward` became a call to it.

### Bug 2: an uninitialised read, found only by Valgrind

Then memcheck on the test suite:

```
==217748== ERROR SUMMARY: 20 errors from 20 contexts (suppressed: 0 from 0)
```

Twenty errors, in a suite that ASan reported completely clean. All the
same shape:

```
==217748== Conditional jump or move depends on uninitialised value(s)
==217748==    at 0x400E45D: party_apply_level (party.c:171)
==217748==    by 0x4003DCC: test_xp_and_levelling (test_combat.c:258)
==217748==    by 0x400DA19: main (test_combat.c:1511)
==217748==  Uninitialised value was created by a stack allocation
==217748==    at 0x4003D46: test_xp_and_levelling (test_combat.c:249)
```

Read it from the bottom: a stack allocation created an uninitialised value,
and `party_apply_level` branched on it. Line 171 is:

```c
if (p->hp > p->max_hp) {
    p->hp = p->max_hp;
}
```

`party_apply_level` clamps HP against the new maximum — so it **reads
`p->hp`** before anything has written it. Line 174 does the same for MP,
which is why the twenty errors land in exactly two places, ten each.

The origin line is the one that names the culprit:
`test_xp_and_levelling (test_combat.c:249)`. That is a stack allocation in
the *test file*:

```c
Player p;              /* nothing initialised */
make_hero(&p, 1, 0);   /* ... which calls party_apply_level, reading p.hp */
```

Eight test functions declare a bare `Player p;` this way. Each gets the
same one-line fix:

```c
Player p;
memset(&p, 0, sizeof p);
```

That takes the suite to zero:

```
==216997== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

**Twenty to zero.** This is the single best argument for running more than
one tool: the sanitizers had been reporting this code clean for seven
chapters, correctly, because it wasn't the kind of bug they look for.

### The same bug, in the shipped game

Fixing the tests is not the end of it, because a test suite has blind spots
of its own. The suite never calls `entity_create_player` — that function
reads a name from `stdin`, so it isn't something the tests can drive. Which
means memcheck on `run_tests` will never say a word about it.

So run the *game* under memcheck instead — driven by the same pty script
from the monkey test, because as we saw, a plain pipe will not do it:

```bash
make
valgrind --track-origins=yes ./game     # or under the driver script
```

```
==218073== Conditional jump or move depends on uninitialised value(s)
==218073==    at 0x400A101: party_apply_level (party.c:171)
==218073==    by 0x400771D: entity_create_player (entity.c:166)
==218073==    by 0x4004A9D: game_create (game.c:926)
==218073==    by 0x40023B2: main (main.c:17)
==218073==  Uninitialised value was created by a stack allocation
==218073==    at 0x40075CF: entity_create_player (entity.c:130)
```

There it is, in shipped code, reached on the very first line of a new game:

```c
Player p;                    /* nothing initialised */

/* ... name, position, xp, gold ... */
party_apply_level(&p);       /* reads p.hp -- which has never been written */
p.hp = p.max_hp;             /* assigned only afterwards */
```

It has been there since Chapter 16, through every clean ASan run, for seven
chapters.

Is it harmful? In practice, almost certainly not — the garbage value gets
overwritten on the next line. But it is undefined behaviour, the compiler is
entitled to assume it never happens, and the *optimizer* is entitled to act
on that assumption in ways that are hard to predict. "It works at `-O0`" is
not a defence; Chapter 10 retired that argument.

The fix is the same one line, plus a documented contract:

```c
/* Zero the whole struct before anything reads it. party_apply_level
   clamps hp against max_hp, so it reads hp -- and reading an
   uninitialised local is undefined behaviour, not merely untidy.
   Valgrind found this in Chapter 22; ASan cannot see it. */
Player p;
memset(&p, 0, sizeof p);
```

And the requirement stated where it exists, in `party.h`:

```c
/* Requires p->hp and p->mp to already hold meaningful values, because it
   clamps them against the new maximums. Passing a Player whose hp/mp
   have never been written is undefined behaviour -- zero the struct
   first. */
void party_apply_level(Player *p);
```

Take the lesson twice over: the first tool you reach for has a blind spot,
and so does the *program you point it at*. A green test suite under
Valgrind means the code the tests reach is clean. It says nothing about the
code they don't.

### Reading a report you didn't write

Now the whole game under Valgrind — 700 random keypresses, ncurses and all:

```
==169928== LEAK SUMMARY:
==169928==    definitely lost: 0 bytes in 0 blocks
==169928==    indirectly lost: 0 bytes in 0 blocks
==169928==      possibly lost: 201 bytes in 3 blocks
==169928==    still reachable: 198,490 bytes in 385 blocks
==169928== ERROR SUMMARY: 3 errors from 3 contexts
```

Three errors. Before assuming they're yours, read the stack traces:

```
==169928==    at 0x4875D13: calloc (vg_replace_malloc.c:1675)
==169928==    by 0x48BC7AA: tparm_setup.lto_priv.0 (lib_tparm.c:657)
==169928==    by 0x48CAA08: _nc_tiparm (lib_tparm.c:1340)
==169928==    by 0x48A6418: newterm_sp (lib_newterm.c:344)
==169928==    by 0x489EBEF: initscr (lib_initscr.c:94)
==169928==    by 0x400AF1C: render_init (render_ncurses.c:6)
==169928==    by 0x4002506: main (main.c:55)
```

Every frame above `render_init` is inside ncurses. This is ncurses
allocating global terminal state in `initscr` and keeping it for the life
of the process — `endwin()` does not release all of it, by design.

The four leak categories are worth learning properly:

- **definitely lost** — nothing points at this block. A real leak. **Zero
  here, which is the number that matters.**
- **indirectly lost** — only reachable through something definitely lost.
  Fix the parent and these go too.
- **possibly lost** — a pointer exists but into the middle of the block,
  not its start. Usually a library storing an interior pointer.
- **still reachable** — a live pointer exists at exit. Not a leak in the
  useful sense; just memory not freed before the program ended.

Zero definitely-lost, across every `map_create`, `world_create`,
`inventory_add`, `realloc` and save-load in a 700-key session. The paired
`*_create`/`*_destroy` discipline from Chapter 10 held.

### Suppressing what isn't yours

Noise you can't fix is worse than useless — it trains you to skim a report
you should read closely. Valgrind takes a suppression file:

```
{
   ncurses-initscr-tparm-setup
   Memcheck:Leak
   match-leak-kinds: possible,reachable
   ...
   fun:initscr
   ...
}
```

`...` matches any number of frames, so this says "any possible-or-reachable
leak whose stack passes through `initscr`". The name is arbitrary and
appears in `-s` output.

Suppressions are a sharp tool: they hide real bugs just as effectively as
fake ones. Write them narrowly — naming the specific library function, not
whole categories — and never suppress `definitely lost`.

```
==170095==    definitely lost: 0 bytes in 0 blocks
==170095==    indirectly lost: 0 bytes in 0 blocks
==170095==      possibly lost: 0 bytes in 0 blocks
==170095==         suppressed: 190,818 bytes in 140 blocks
==170095== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 3 from 3)
```

A clean report you can trust, and one you'll actually read next time.

## Compile and run

```bash
make clean
make test
```

```
301314 checks, 0 failed
```

```bash
make valgrind
```

```
==170229== All heap blocks were freed -- no leaks are possible
==170229== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

```bash
make fuzz-sanitize 2>/dev/null
```

```
fuzzed 4000 random files through 5 parsers: no crash
level 1 hp still 20 (expect 20)
spell count still 13 (expect 13)
```

```bash
make sanitize
./game
```

Then, the whole game under memcheck, played for real:

```bash
make
valgrind --leak-check=full --track-origins=yes \
         --suppressions=valgrind.supp ./game
```

It will feel slow — that's thirty-odd times the work per instruction. Play
for a few minutes, visit a shop, fight something, sleep at the inn, quit
properly. Anything it reports from a file in *your* project is yours.

## Common errors

**Assuming clean sanitizers means correct:**

Twenty uninitialised reads survived seven chapters of clean ASan runs.
Each tool has a scope; "no diagnostics" only means "none of the bugs this
tool looks for."

**Side effects inside `assert`:**

```c
assert(inventory_add(inv, id, 1));
```

Works in development, silently does nothing in a `-DNDEBUG` build, and the
item never arrives. Assert on values, never on actions.

**Testing for overflow after it happens:**

```c
if (p->xp + xp > PARTY_MAX_XP) { ... }
```

The addition in the condition is the overflow. Rearrange to
`xp > PARTY_MAX_XP - p->xp`.

**Suppressing too broadly:**

```
{
   everything
   Memcheck:Leak
   ...
}
```

Silences the report entirely, including your own bugs, forever. Name the
library function.

**Reading only the top frame:**

The first line of a Valgrind stack is usually inside `malloc` or a library.
Scan down for the first file that belongs to you. In the ncurses case there
isn't one above `render_init` — which is the answer.

## Exercises

1. Reintroduce the uninitialised read: remove the `memset` from
   `entity_create_player` and run `make valgrind`. How many errors come
   back — and why is that number surprising? Then run the *game* under
   memcheck and count again. Finally run `make sanitize` and play — does
   ASan say anything?
2. Add an assertion to `combat_base_damage` stating that the result is
   never below 1. Then temporarily delete the floor and run the tests.
   Which fires first, the assertion or the existing test — and which
   message would you rather debug from?
3. Build with `-DNDEBUG` and confirm the assertions are gone (try
   deliberately violating one). Should this game ship with assertions
   enabled or disabled? Argue both sides: what does a player gain from a
   crash, versus from continuing with a corrupted state?
4. *Open-ended:* The fuzzer generates random *files*. Sketch a fuzzer for
   random *save files specifically* — structurally valid `key = value`
   lines with random values. Why would that find different bugs than the
   character-level fuzzer, and which is more likely to find a real one?

<details>
<summary>Solutions</summary>

1. **Zero**, and that is the whole point of the exercise. `make valgrind`
   runs `run_tests`, and the test suite never calls
   `entity_create_player` — the function reads a name from `stdin`, so
   there is no way for a test to drive it. A tool can only report on code
   that actually executes.

   Run the game instead — under the pty driver, or just play it and quit
   properly — and the bug appears immediately, before you have taken a
   single step:

   ```bash
   valgrind --track-origins=yes ./game
   ```

   ```
   ==225437== ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)
   ```

   Two errors — `party.c:171` and `party.c:174`, the clamps that read `hp`
   and `mp`, both originating at the bare `Player p;` in
   `entity_create_player`.

   ASan, meanwhile, says nothing at all: the game plays normally, reports
   no diagnostics, and behaves exactly as it always has. That contrast,
   reproduced by hand, is the most convincing possible argument for
   running both tools — and for remembering that "the test suite is
   clean" and "the program is clean" are different claims.

2. The assertion fires first — but only just, and the comparison is more
   interesting than "assertions win."

   With the assertion in place, the run ends immediately:

   ```
   run_tests: combat_math.c:21: combat_base_damage: Assertion `base >= 1' failed.
   ```

   Without it, the same deleted floor produces:

   ```
   base damage
     FAIL test_combat.c:30: combat_base_damage(5, 0, 40) == -15, expected 1
   ...
   301314 checks, 3 failed
   ```

   Each tells you something the other doesn't. The assertion names the
   line *inside the function that computed the bad value* — you don't
   have to go looking for it. The test names **the inputs and the wrong
   answer** (`(5, 0, 40)` gave `-15`), which the assertion never shows,
   and it keeps going: three checks fail, so you learn the floor is
   load-bearing in three places rather than one.

   Note also what the assertion costs: `abort()` does not flush `stdout`,
   so the buffered progress line telling you *which test* was running can
   be lost. That is worth knowing before you debug at 2am.

   Tests tell you *that* something broke, and with what inputs;
   assertions tell you *where* it broke, at the moment it broke. Neither
   replaces the other — which is why the answer is to keep both.

3. With `-DNDEBUG` the asserts compile to nothing and a violated
   invariant proceeds silently. For a single-player game whose worst
   failure is a lost session, shipping with assertions **enabled** is
   defensible — a crash with a file and line produces a fixable bug
   report, while continuing produces a corrupted save the player keeps
   using. For software where a crash costs more than wrong behaviour (a
   flight controller, a database mid-write), the calculus reverses. The
   real answer is that this is a product decision, not a technical one,
   and the common practice of "assertions off in release" is a default
   rather than a law.

4. A structural fuzzer generates files that get *past* the tokeniser and
   into the semantic checks — `level = -2147483648`, `item = 5 0`,
   duplicate keys, a `version` line appearing twice, fields in unexpected
   order. The character-level fuzzer mostly exercises rejection paths,
   because random bytes rarely form a valid `key = value`. The structural
   one is far more likely to find a real bug, because the interesting
   logic is *after* parsing succeeds. The general principle: fuzz at the
   layer where the logic is, not just at the front door. (Both are worth
   having; the cheap one runs first.)

</details>

## Next up

The game is solid: no leaks, no undefined behaviour, no uninitialised
reads, 301,314 assertions passing, and a suppression file so the report
stays readable. What it does not have is an ending.

**Checkpoint F comes next** — the last one. Before Chapter 23 you'll decide
how the story finishes, which of the optional dungeons ship, and whether
the course covers packaging the game so somebody else can play it.
