# Chapter 3: Decisions and Loops

## Where we are

Your game asks for a name and prints a character sheet, then stops — every
line of it runs exactly once, top to bottom, no matter what. Chapter 2 gave
the game data to remember; this chapter gives it a pulse. By the end,
you'll have a one-round coin-flip duel: the player calls heads or tails,
the game keeps asking until it gets a real answer, and a winner is
declared.

## The problem

Right now, `main` cannot make a decision, cannot repeat itself, and cannot
be broken into named, reusable pieces. Every real program needs all three:
branching logic (`if`, `switch`), repetition (`while`, `for`), and
organization (functions). This chapter covers all four control-flow tools
plus the mechanics of functions — parameters, return values, and *scope*,
which is what keeps a variable in one function from stepping on a
same-named variable in another.

## C concept spotlight

### Making decisions: `if`/`else` and `switch`

```c
char call = 'h';

if (call == 'h') {
    printf("You called heads.\n");
} else if (call == 't') {
    printf("You called tails.\n");
} else {
    printf("That's not a coin side.\n");
}
```

`if` evaluates a condition; if it's true (nonzero), the block runs. `else
if` chains additional conditions, checked only if every earlier one failed.
The final bare `else` catches everything else. Note `==` for comparison,
not `=` — `=` is assignment. Mixing these up is one of C's most notorious
beginner traps; the "Common errors" section shows you exactly what it looks
like when it goes wrong.

`switch` does the same kind of job when you're comparing one variable
against several exact values:

```c
switch (call) {
    case 'h':
        printf("(switch) Heads it is.\n");
        break;
    case 't':
        printf("(switch) Tails it is.\n");
        break;
    default:
        printf("(switch) Unrecognized call.\n");
        break;
}
```

Each `case` is an entry point, not a boundary — without `break`, execution
*falls through* into the next `case` regardless of whether its value
matches. That's *occasionally* useful (you'll use it on purpose in this
chapter's actual duel code, to treat `'h'` and `'H'` identically) and
__frequently__ a bug when forgotten. `default` runs when nothing else matched;
it's optional but good practice to include.

### Repeating work: `while` and `for`

```c
printf("Countdown with for:\n");
for (int i = 3; i >= 1; i--) {
    printf("  %d...\n", i);
}
```

A `for` loop bundles three things in its parentheses, separated by
semicolons: a **setup** that runs once (`int i = 3`), a **condition**
checked before every iteration (`i >= 1`), and a **step** that runs after
every iteration (`i--`, meaning "decrease `i` by 1"). Use `for` when you
know roughly how many times you're looping, or you're counting through a
range.

```c
printf("Retry with while:\n");
int attempts = 0;
int success = 0;
while (!success && attempts < 3) {
    attempts++;
    printf("  Attempt %d\n", attempts);
    if (attempts == 2) {
        success = 1;
    }
}
printf("Succeeded after %d attempt(s).\n", attempts);
```

`while` just repeats its block as long as its condition stays true, with no
built-in setup or step — you manage those yourself. `!success` reads as
"not success"; since C has no dedicated boolean type in the version we're
using (you may see `<stdbool.h>` in other people's code — this course
sticks with plain `int`, where `0` means false and *anything else* means
true), `!success` is true exactly when `success` is `0`. Use `while` when
the number of iterations depends on something that can only be known while
you're looping — exactly the shape of "keep asking until the player types
something valid," which is what the duel needs.

### Organizing code: functions, parameters, and scope

```c
int square(int n)
{
    int result = n * n;
    return result;
}

int main(void)
{
    int result = 9;   /* same name as the local inside square() on purpose */
    printf("Before: result = %d\n", result);
    printf("square(4) = %d\n", square(4));
    printf("After:  result = %d\n", result);
    return 0;
}
```

```
Before: result = 9
square(4) = 16
After:  result = 9
```

`square` takes one **parameter** (`n`), does some work, and **returns** a
value with `return`. Notice `main`'s `result` and `square`'s `result` don't
interfere with each other at all — each is scoped to the function it's
declared in. **Scope** is the region of code where a variable's name is
valid; a variable declared inside a function (a *local* variable) simply
doesn't exist as far as any other function is concerned, even one with the
exact same name. This is a feature, not a coincidence — it's why you can
freely name a loop counter `i` in ten different functions without them
colliding (but please feel free to make them more descriptive than that).

You've been calling functions since Chapter 1 (`printf`, `fgets`) without
writing your own. The shape is always the same: a return type, a name,
parameters in parentheses, and a body in braces. `void` as a return type
means "returns nothing"; you'll write one of those shortly.

## Apply it

Add three functions above `main`, then call the last one from `main`.

`flip_coin` — deliberately, honestly fake for now:

```c
int flip_coin(void)
{
    /* Not random yet -- always lands on heads. We fix this for real in
       Chapter 15, once you know how to generate random numbers. */
    return 0;   /* 0 = heads, 1 = tails */
}
```

This is bad, and it's supposed to be. A coin that always lands the same way
isn't a coin flip. Real randomness needs library functions and a concept
(seeding) this course hasn't earned yet — Chapter 15 is entirely devoted to
doing this properly, with tests. For now, the game *looks* like it's
flipping a coin, and you know exactly why it isn't really. That gap is the
point: naming a limitation out loud beats hiding it.

`ask_call` — reads one character, validated, retrying on garbage input:

```c
char ask_call(void)
{
    char call = '\0';

    while (call != 'h' && call != 't') {
        printf("Call it: (h)eads or (t)ails? ");

        int typed = getchar();

        switch (typed) {
            case 'h':
            case 'H':
                call = 'h';
                break;
            case 't':
            case 'T':
                call = 't';
                break;
            default:
                break;
        }

        /* Discard the rest of the line, including the newline, so the
           next trip through the loop starts clean. */
        while (typed != '\n' && typed != EOF) {
            typed = getchar();
        }

        if (call != 'h' && call != 't') {
            printf("Didn't catch that -- type h or t.\n");
        }
    }

    return call;
}
```

`getchar()` is a new library function: it reads exactly one character from
`stdin` and returns it — as an `int`, not a `char`. That's deliberate on
the standard library's part: `getchar` needs a way to signal "there's
nothing left to read" (**`EOF`**, end-of-file), and every possible `char`
value is already a real character, so it borrows a value outside `char`'s
range to mean "no more input." A plain `char` couldn't represent that.
You'll compare `getchar()`'s result directly against character constants
like `'h'` anyway, because C automatically converts between `char` and
`int` where it makes sense.

The inner `while` loop matters: after `getchar()` reads one character, your
Enter key's newline (and anything else you typed) is still sitting unread.
Without draining it, the *next* trip through the outer loop would
immediately "read" that leftover newline instead of waiting for you to type
again — the same family of bug Chapter 2 warned about with `scanf`,
happening here for a different reason.

The two-`case`-one-`break` pattern (`case 'h': case 'H':`) is intentional
fallthrough: both cases run the same body, because falling through an empty
`case` is harmless — it's falling through a `case` with code in it that
usually indicates a missing `break`.

`run_duel` — ties it together:

```c
void run_duel(void)
{
    printf("\n");
    printf("A stranger flips an old coin and grins.\n");

    char call = ask_call();

    printf("The coin spins");
    for (int i = 0; i < 3; i++) {
        printf(".");
    }
    printf("\n");

    int result = flip_coin();
    char landed = 'h';
    if (result == 1) {
        landed = 't';
    }

    if (landed == 'h') {
        printf("It lands on heads. ");
    } else {
        printf("It lands on tails. ");
    }

    if (call == landed) {
        printf("You called it! You win the duel.\n");
    } else {
        printf("Not what you called. You lose the duel.\n");
    }
}
```

`run_duel` returns `void` — it doesn't hand anything back to its caller, it
just does things (prints, calls other functions). Finally, call it from
`main`, right after the character sheet prints:

```c
run_duel();
```

## Compile and run

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
./game
```

Type a name, then `h` or `t` when prompted. Here's the expected output for
name `Elowen`, calling `h`:

```
****************************************
*                                      *
*           UNTITLED RPG               *
*                                      *
****************************************

What is your name, hero? Elowen

Welcome, Elowen
----------------------------------------
HP:     20
MP:     8
ATK:    6
DEF:    4
Rank:   E
Power:  5.0
----------------------------------------

A stranger flips an old coin and grins.
Call it: (h)eads or (t)ails? h
The coin spins...
It lands on heads. You called it! You win the duel.
```

Try typing garbage (like `x`) at the call prompt first — you should see
"Didn't catch that" and get asked again, as many times as it takes.

## What just happened

Control flow in C is really just the CPU deciding, each instruction, where
to go next — normally "the next line," but `if`/`switch` can redirect it
conditionally, and `while`/`for` redirect it *backwards*, re-running
earlier instructions. Every function call is also a jump, but a
self-returning one: the CPU remembers where it was (on a data structure
called the **call stack**, which you'll meet properly once you understand
memory in Chapter 10) and comes back the instant `return` executes. That's
also the mechanism behind scope: `square`'s `result` and `main`'s `result`
occupy *different* memory, allocated fresh each time their function is
entered and discarded the moment it returns — they were never the same
variable, just the same four letters.

## Common errors

**`=` instead of `==` in a condition:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o oops oops.c
oops.c: In function ‘main’:
oops.c:6:9: warning: suggest parentheses around assignment used as truth value [-Wparentheses]
    6 |     if (call = 'h') {
      |         ^~~~
```

This compiles — `call = 'h'` is a valid expression (it assigns `'h'` to
`call` *and* evaluates to `'h'`, which is nonzero, so the `if` always takes
the true branch) — but it is almost never what you meant. `-Wparentheses`
(bundled into `-Wall`) catches the most common shape of this mistake. Read
every warning like this one as "the compiler thinks you made a typo," not
as noise to dismiss.

**Missing `break` in a `switch` — falls through, and the compiler notices:**

```c
switch (call) {
    case 'h':
        printf("Heads!\n");
    case 't':
        printf("Tails!\n");
        break;
}
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o ft ft.c
ft.c: In function ‘main’:
ft.c:7:13: warning: this statement may fall through [-Wimplicit-fallthrough=]
    7 |             printf("Heads!\n");
      |             ^~~~~~~~~~~~~~~~~~
ft.c:8:9: note: here
    8 |         case 't':
      |         ^~~~
```

Calling this with `call == 'h'` still prints *both* lines — the warning
doesn't stop the compile, it just tells you your code is legal but
suspicious:

```
Heads!
Tails!
```

`-Wextra` catches most accidental fallthrough, which is exactly why
`ask_call`'s *intentional* fallthrough (`case 'h': case 'H':`) doesn't
trigger this warning — two `case` labels stacked with nothing between them
is recognized as deliberate, while a `case` with a real statement and no
closing `break` is what the compiler flags. Still: don't rely on the
warning catching every case. Get in the habit of asking, every time you
write a `case`, "did I mean to fall through here?"

**An infinite loop — the classic missing-update bug:**

```c
int attempts = 0;
while (attempts < 3) {
    printf("Attempt\n");
    /* forgot: attempts++; */
}
```

This compiles and runs — and never stops, printing `Attempt` forever, until
you kill it with Ctrl+C. `while`'s condition is checked every iteration,
but nothing here ever changes `attempts`, so it's `0 < 3` — forever true —
every single time. If a program you run in this course seems to hang, this
family of bug (a loop condition that never becomes false) is the first
thing to suspect.

## Exercises

1. In `ask_call`, what happens if you type more than one character before
   pressing Enter — for example, typing `hello` instead of `h`? Trace
   through the code by hand first, then try it and see if you predicted
   correctly.
2. Add a third loop iteration variant: rewrite the `for` countdown as a
   `while` loop that does the exact same thing. Which version do you find
   more readable, and why do you think `for` exists as a separate construct
   at all instead of everyone just using `while`?
3. `run_duel` currently only supports a heads/tails duel. Add a
   `best_of_three` variant conceptually (you don't have to fully build it):
   what would need to change about `flip_coin`'s fakeness for a
   best-of-three duel to feel meaningfully different from calling
   `run_duel` three times in a row? (You're not expected to solve the
   randomness problem — just identify what breaks.)
4. *Open-ended:* This chapter's duel has exactly one possible outcome per
   run, since the coin is fixed. If you could add one more `if`/`switch`
   branch to make the duel feel more like *your* game's tone (see
   Checkpoint B, coming soon) — a taunt, a special outcome on a specific
   call — what would it say?

<details>
<summary>Solutions</summary>

1. `getchar()` only ever reads the *first* character you typed — here,
   `'h'`. The switch matches it and sets `call = 'h'` immediately. The
   inner `while (typed != '\n' && typed != EOF)` loop then discards
   everything else on the line (`ello` and the newline) one character at a
   time via repeated `getchar()` calls, so the extra letters are silently
   thrown away rather than causing an error or getting reinterpreted as a
   second call. The duel proceeds exactly as if you'd typed just `h`.

2. ```c
   int i = 3;
   while (i >= 1) {
       printf("  %d...\n", i);
       i--;
   }
   ```
   Functionally identical output. `for` exists because "set up a counter,
   check it, step it" is such a common pattern that bundling all three
   parts on one line makes the loop's *entire lifecycle* visible at a
   glance — with `while`, the setup and the step can be far apart (or,
   per the infinite-loop error above, accidentally missing entirely),
   which is a common source of bugs `for` structurally prevents.

3. The core problem: `flip_coin` always returns the same result, so
   calling `run_duel()` three times wouldn't produce three independent
   coin flips — it would produce the same flip three times, making
   "best of three" meaningless (you'd win 3-0 or lose 0-3, never
   anything else). A real best-of-three needs `flip_coin` to genuinely
   vary between calls, which is precisely the randomness problem this
   chapter deferred to Chapter 15. This exercise has no code to write yet
   — the honest answer is "this feature is blocked on a concept you don't
   have," which is itself a useful thing to be able to recognize.

4. No fixed answer — this is a taste exercise. Keep whatever you come up
   with; Checkpoint B is coming up in a few chapters and will ask you to
   think about tone more formally.

</details>

## Next up

`run_duel`, `ask_call`, and `flip_coin` all live in one file right now, and
that file is about to get crowded. Chapter 4 splits your project across
multiple files — headers, source files, and a hand-written `Makefile` — and
explains, properly this time, what the compiler and linker are doing when
more than one `.c` file is involved.
