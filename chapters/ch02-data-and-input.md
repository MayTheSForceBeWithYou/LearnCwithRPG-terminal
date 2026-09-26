# Chapter 2: Data and Input

## Where we are

Chapter 1 gave you a title screen that prints the same static text every
time. You understand `main`, `printf`, and the pipeline that turns your
source into a running program. What you don't have yet is any way for the
game to remember *anything* — a name, a number, a single character. This
chapter fixes that: you'll learn C's basic types, read a real line of
keyboard input, and watch your hero's name and starting stats appear on
screen.

## The problem

An RPG that can't ask your name isn't much of an RPG. You need somewhere to
put the answer once you have it, and you need a safe way to actually get it
from the keyboard without either crashing on long input or reading
`stdin` (the keyboard, from the program's point of view) wrong.

## C concept spotlight: types, casting, and how to read a line

### The three types you'll use constantly

```c
int   hp    = 20;   /* a whole number */
char  rank  = 'E';  /* a single character */
float power = 5.0f; /* a number with a fractional part */
```

- **`int`** holds a whole number — no fractional part. On the machine this
  course targets (a 64-bit Linux system), that's 32 bits of storage, giving
  a range of roughly -2.1 billion to +2.1 billion. Plenty for HP, gold,
  experience points.
- **`char`** holds a single character — but under the hood, it's just a
  small integer. `'E'` isn't magic; it's shorthand for the number 69, which
  is `'E'`'s position in **ASCII**, the standard mapping from small integers
  to the letters, digits, and symbols. `printf("%c", rank)` prints the
  *character*; `printf("%d", rank)` would print `69`. Same bits, different
  interpretation — you'll see that theme again and again in C.
- **`float`** holds a number with a fractional part, stored in a format
  that trades some precision for a huge range. `5.0f` — note the trailing
  `f` — tells the compiler "this literal is a `float`," not the wider
  `double` type it would otherwise default to. You'll mostly see `float`
  in this course; `double` shows up if you ever need more precision than a
  `float` gives.

Each of these has a size in bytes — the exact number of bytes any variable
or type occupies is available via the `sizeof` operator, which you'll use
in a moment to safely size a text buffer.

### The bug that makes casting matter

Here's a five-line trap that catches nearly everyone once:

```c
int current_hp = 15;
int max_hp = 20;

int percent = current_hp / max_hp * 100;
printf("%d%%\n", percent);   /* prints 0%, not 75% */
```

`current_hp / max_hp` is `15 / 20`. Both operands are `int`, so C performs
**integer division**: it computes the mathematically exact answer (0.75)
and then throws away everything after the decimal point, giving `0`. Then
`0 * 100` is still `0`. The percentage you wanted — 75% — never had a
chance to exist; it was destroyed a step before the multiplication even
ran.

The fix is an explicit **cast**: telling the compiler to treat a value as a
different type for one expression.

```c
float percent = (float)current_hp / max_hp * 100;
printf("%.0f%%\n", percent);   /* prints 75% */
```

`(float)current_hp` converts just that one value to a `float` *before* the
division happens. Once one side of `/` is a `float`, C promotes the other
side to match, so the division is now real (fractional) division, not
integer division. Try both versions yourself:

```c
#include <stdio.h>

int main(void)
{
    int current_hp = 15;
    int max_hp = 20;

    int bad_percent = current_hp / max_hp * 100;
    float good_percent = (float)current_hp / max_hp * 100;

    printf("Integer division:  %d%%\n", bad_percent);
    printf("With a cast:       %.0f%%\n", good_percent);

    return 0;
}
```

```
Integer division:  0%
With a cast:       75%
```

This isn't a rare edge case — it's one of the single most common bugs in C
and C-derived languages, and you now know both why it happens and how to
fix it. (Notice `%.0f` above, and `%.1f` later — the number after the `.`
in a float format specifier controls how many digits after the decimal
point get printed. `%.0f` rounds to a whole number; `%.1f` shows one digit
of precision.)

### Reading a line of input: `fgets`, not `scanf`

You'll see `scanf` in other C tutorials and almost certainly in older code.
This course doesn't use it, for reasons worth knowing:

- `scanf("%s", name)` (no size limit given) has no idea how big `name` is,
  so it will happily write past the end of your buffer if the typed input
  is longer than you expected — a classic, dangerous **buffer overflow**.
- Reading anything that *isn't* a string (`scanf("%d", &age)`) requires the
  `&` operator, which means "the address of this variable." That's the
  address-of operator, the foundation of pointers — a big topic this course
  gives its own chapter (7). Using it correctly right now, without that
  foundation, would be like invoking a spell you don't understand yet.
- `scanf` leaves the newline character from your Enter key sitting unread
  in the input stream, which then silently corrupts the *next* read if
  you're not careful. It's a well-known trap even for people who've used
  `scanf` for years.

`fgets` sidesteps all three problems for the case that matters right now:
reading one line of text.

```c
char name[64];
printf("What is your name, hero? ");
fgets(name, sizeof name, stdin);
```

Don't worry about the `[64]` yet — that's an array, and arrays get a full
chapter (5) soon, including exactly how indexing and bounds work. For now,
read `char name[64]` as "a fixed-size box, 64 bytes, for text." `fgets`
takes three arguments: where to put the text (`name`), the maximum number
of bytes it's allowed to write (`sizeof name` — the exact size of that box,
computed automatically so you never have to hardcode it), and where to read
from (`stdin`, the keyboard). Unlike `scanf`, `fgets` will never write past
the end of your buffer, no matter what the user types — it stops at 63
characters and adds a terminating byte itself. (You'll learn exactly what
that terminating byte is and why it matters in Chapter 12.)

One quirk worth knowing now: if what you typed fits, `fgets` keeps the
newline character from pressing Enter as part of the string. That's why the
game code below prints `"Welcome, %s"` with no `\n` of its own — the
newline you typed is already sitting at the end of `name`.


### Types do not enforce game rules

`int hp = -5` is a perfectly valid `int`. The type system will not save you.
Clamping HP to `[0, max_hp]` is **game logic** — practice `04_clamp_hp`
before you expect battle code to do it later.


## Apply it

Pick up `main.c` where Chapter 1 left it. Replace the last two `printf`
calls (the placeholder-title note) with a name prompt and a character
sheet:

```c
char name[64];
printf("What is your name, hero? ");
fgets(name, sizeof name, stdin);

int hp = 20;
int mp = 8;
int attack = 6;
int defense = 4;
char rank = 'E';
float power_rating = (float)(attack + defense) / 2.0f;

printf("\n");
printf("Welcome, %s", name);
printf("----------------------------------------\n");
printf("HP:     %d\n", hp);
printf("MP:     %d\n", mp);
printf("ATK:    %d\n", attack);
printf("DEF:    %d\n", defense);
printf("Rank:   %c\n", rank);
printf("Power:  %.1f\n", power_rating);
printf("----------------------------------------\n");
```

The starting stats (`20`/`8`/`6`/`4`/`'E'`) are placeholders, same spirit as
the placeholder title — real balance numbers are a long way off and not
worth agonizing over yet. `power_rating` exists specifically to give you a
harmless place to practice the cast from the spotlight section, on real
game data instead of a throwaway example.

## Compile and run

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
./game
```

Type a name and press Enter. Expected output (typing `Elowen`):

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
```

Zero warnings from `gcc`, same as always.

## What just happened

Each variable you declared reserved a fixed amount of memory — `sizeof`
would report 4 bytes for each `int`, 1 for the `char`, 4 for the `float`,
64 for `name`'s buffer — and each `printf` format specifier (`%d`, `%c`,
`%f`, `%s`) told `printf` how to reinterpret the bits sitting in that
memory as something printable. That reinterpretation is exactly why format
specifiers have to match their arguments: `printf` trusts you completely
about what type is coming, and has no way to check at compile time on its
own. `-Wformat` (bundled into `-Wall`) *can* check, at compile time, most of
the time — which is exactly why you never turn these warnings off. See the
first "Common error" below for what happens when that check is the only
thing standing between you and a crash.

## Common errors

### A mismatched format specifier — caught by the compiler:

If you change
`printf("HP:     %d\n", hp);`
to
`printf("HP:     %s\n", hp);`
and recompile with
```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
```
you'll see something like this:

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
main.c: In function ‘main’:
main.c:26:22: warning: format ‘%s’ expects argument of type ‘char *’, but argument 2 has type ‘int’ [-Wformat=]
    6 |     printf("HP: %s\n", hp);
      |                 ~^     ~~
      |                  |     |
      |                  |     int
      |                  char *
      |                 %d
```

This is only a *warning* — `gcc` still produced a program. Running it is a
different story:

```bash
$ ./game
[nothing printed]
$ echo $?
139
```

or

```bash
$ ./game
zsh: exec format error: ./game
$ echo $?
126
```

Exit status 126 means that the command was found, but it could not be executed.

Exit status 139 means the operating system killed the program with signal
11 — a **segmentation fault**, the OS's way of saying "you tried to access
memory you have no right to." `%s` told `printf` "the next argument is an
address; go read a string starting there." The `int` value `20` isn't a
valid address at all, so `printf` crashed trying to read from it. This is
**undefined behaviour**: the C standard puts no requirement on what happens
when you do this, so *anything* could result — a crash (what you saw here),
silently wrong output, or apparent success that fails later on a different
machine or compiler. The compiler's warning was your only warning; heed it.

### A buffer too small for the input — `fgets` degrades safely:

```c
char name[4];
printf("Name: ");
fgets(name, sizeof name, stdin);
printf("Hi %s", name);
```

Typing `Elowen` and pressing Enter:

```
Name: Hi Elo
```

No crash, no overflow — `fgets` filled all 4 bytes it was allowed to
(`'E'`, `'l'`, `'o'`, and a terminating byte) and simply stopped, dropping
the rest of what you typed. Compare this to the `scanf`/buffer-overflow
danger described in the spotlight section: this is exactly the failure mode
`fgets`'s size argument exists to prevent. The lesson isn't "always size
buffers exactly right" (you can't always predict input length) — it's "give
`fgets` a real size and it will never write past it, no matter what."

### Forgetting the `f` suffix on a float literal (not an error, but a common `-Wpedantic`-adjacent surprise):

```c
float power = 5.0;   /* compiles fine, no warning here */
```

This one actually compiles silently — `5.0` is a `double` literal, and C
converts it to `float` automatically when you assign it. You'll get away
with this constantly. It matters more once you're *passing* a `float` value
directly into a function expecting exactly `float` rather than assigning
it, which you won't do until later chapters — filed away for when it's
relevant.

## Exercises

> **Practice drills:** `code/ch02/practice/` before exercise 2.

1. *Practice.* Finish format specs, int division, casted average, HP clamp.
2. *Reading.* Assign `hp = -5` in the chapter program — what prints? Why is
   that a logic problem, not a type error? (Restore afterward.)
3. *Precedence.* Remove the `(float)` cast from `power_rating` as
   `attack + defense / 2`. Is the bug integer division, precedence, or both?
4. *Open-ended:* Sketch a `level` field type for "Elowen, Level 1 Wanderer."

<details>
<summary>Solutions</summary>

1. Practice `solutions/`.
2. Prints `-5`. Guard at modification sites later.
3. Both: `/` binds tighter than `+`, and int division truncates.
4. `int level` (or similar).

</details>

## Next up

Right now your "game" runs top to bottom once and stops. Chapter 3 gives it
a pulse: `if`, loops, and functions, building toward your first actual
interaction — a one-round coin-flip duel with a real winner and loser.
