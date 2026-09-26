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

## C concept spotlight: integer types, signed vs unsigned, and type selection

### The basic types

```c
int            hp        = 20;     /* a whole number, signed */
unsigned int   gold      = 0;      /* whole number, no negatives */
char           rank      = 'E';    /* a single character */
float          power     = 5.0f;   /* fractional number */
```

Every variable in C has a **type**, which determines three things: how many
bytes it occupies, what range of values it can represent, and how
operations on it behave.

### `int` — the workhorse integer type

**`int`** holds a **signed** whole number. On the machine this course
targets (a 64-bit Linux system running GCC), that's 32 bits of storage,
giving a range of roughly -2.1 billion to +2.1 billion. More than enough
for HP, gold, experience points, damage values, and loop counters.

`int` is C's **default integer type**, and that's not an accident. The C
standard says `int` should be "the natural size suggested by the
architecture" — on x86-64, that's 32 bits, which fits in a single register
and is the native width for most arithmetic instructions. Operations on
`int` are typically the fastest integer operations the CPU offers.

**When to use `int`:** Stack-local variables, function parameters,
arithmetic intermediates, loop counters, and return values. If you're
unsure, `int` is usually the right call. The performance and codegen
characteristics almost always justify the extra bytes over smaller types
like `short` or `char`.

### `unsigned int` — for values that never go negative

**`unsigned int`** (often abbreviated `unsigned`) is also 32 bits, but its
range is 0 to ~4.3 billion — the sign bit is repurposed as a value bit.

Use `unsigned` for **counts, sizes, indices, and game resources that are
conceptually non-negative**: HP, MP, gold, inventory counts, array
capacities. Choosing `unsigned` documents intent: this value should never
be negative, and if it somehow becomes negative through a bug, it will wrap
to a huge positive value (4,294,967,295 for a -1), which is much more
likely to be caught than a small negative number that slips through
unnoticed.

**The wraparound rule:** Unsigned overflow is well-defined in C — it wraps
modulo 2^N. If you subtract 1 from an `unsigned` holding 0, the result is
the maximum value. This is **not undefined behaviour** (unlike signed
overflow), but it is almost certainly a logic bug in game code. Subtraction
from unsigned values requires the same care as division by zero: validate
your inputs.

### `char` — characters are small integers

**`char`** holds a single character, but under the hood it's an 8-bit
integer. `'E'` is not magic; it's syntactic sugar for the number 69,
`'E'`'s position in **ASCII** (the standard mapping from integers 0–127 to
letters, digits, and symbols). `printf("%c", rank)` prints the *character*;
`printf("%d", rank)` would print `69`. Same bits, different interpretation
— you will see this theme everywhere in C.

**Caution:** Whether `char` is *signed* or *unsigned* by default is
**implementation-defined** (meaning GCC on x86-64 can choose differently
than Clang on ARM). If you are using `char` for arithmetic rather than
text, write `signed char` or `unsigned char` explicitly so you get the
behaviour you expect on every compiler.

### `float` — fractional numbers

**`float`** stores a number with a fractional part using IEEE 754
binary32: ~7 decimal digits of precision, exponents from 10^-38 to 10^38.
`5.0f` — note the trailing `f` — tells the compiler this literal is a
`float`, not the wider `double` (64-bit, ~15 digits) it would otherwise
assume.

You'll mostly see `float` in this course. `double` appears when you need
more precision than `float` provides, which for an RPG is almost never.

### Fixed-width types: when you need an exact size

`<stdint.h>` provides types with **guaranteed sizes**, regardless of
platform:

```c
#include <stdint.h>

uint8_t   small_count = 0;     /* exactly 8 bits,  range 0..255 */
uint16_t  item_id     = 1042;  /* exactly 16 bits, range 0..65535 */
int32_t   damage      = -50;   /* exactly 32 bits, signed */
```

**When to use fixed-width types:**

1. **File formats and network protocols.** If you are writing data to disk
   or sending it over a wire, you need to know the exact byte layout.
   `int` could be 16, 32, or 64 bits depending on platform; `int32_t` is
   always 32.

2. **Arrays or structs with many elements.** An array of 10,000 HP values
   stored as `uint16_t` (0..65535) costs 20KB; as `int` (32 bits each), it
   costs 40KB. If HP never exceeds 9999 and you are storing thousands of
   them, the savings are real.

3. **Bit-manipulation.** If you are setting and clearing individual bits
   (flags, bitmasks), knowing the exact width makes the logic clearer and
   the operations faster.

**When NOT to use them:** Single local variables, function parameters, or
arithmetic intermediates. A local `uint8_t counter` does not save you a
byte — alignment rules and integer promotion (below) mean it still occupies
a register and gets widened to `int` before every arithmetic operation.

### Integer promotion and why `int` wins for locals

C's **integer promotion rules** say that any integer type *smaller than
`int`* is promoted to `int` before arithmetic. This means:

```c
uint8_t a = 200;
uint8_t b = 100;
uint8_t sum = a + b;   /* a and b promoted to int, added, truncated back */
```

The addition does not happen at 8-bit width. Both `a` and `b` are widened
to `int` (32 bits), added there, and the result is truncated back to 8 bits
when stored. You pay the cost of two promotions and a truncation, and the
compiler *still* has to allocate a register or stack slot, which on x86-64
is 8 bytes due to alignment.

For a **single local variable or parameter**, using `uint8_t` instead of
`int` buys you nothing but extra conversions. Use `int` or `unsigned` for
locals unless you have measured a reason not to.

### Alignment and padding: why smaller does not always mean smaller

Every type has an **alignment requirement** — the addresses where it can
legally live in memory. On x86-64:

- `char` / `uint8_t` can live at any address (alignment 1)
- `int` / `uint32_t` must start at an address divisible by 4 (alignment 4)
- `double` / `uint64_t` must start at an address divisible by 8 (alignment 8)

The compiler inserts **padding** (invisible, unused bytes) to satisfy these
rules. Consider:

```c
struct {
    char  a;      /* 1 byte */
    /* 3 bytes of padding inserted here */
    int   b;      /* 4 bytes */
    char  c;      /* 1 byte */
    /* 3 bytes of padding at the end */
};
/* sizeof this struct: 12 bytes, not 6 */
```

If you replace `int b` with `uint8_t b`, you do **not** get a 3-byte
struct. You get an 8-byte struct with different padding, because `c` still
needs space and the struct's overall alignment is still the maximum of its
members'. Shrinking types to save memory only works when you shrink *many*
of them in an array or pack them carefully in a struct.

**Practice `05_sizeof_alignment`** before writing any struct that packs
data tightly.

### Signed vs unsigned: overflow and undefined behaviour

Signed overflow (e.g., `INT_MAX + 1`) is **undefined behaviour**. The
compiler is allowed to assume it never happens, which lets it optimize
aggressively — but also means if it *does* happen, anything can result:
wrap, trap, or time travel. This is not academic: you will see real bugs
caused by this in Chapter 22 when you run your game under `-fsanitize=undefined`.

Unsigned overflow is **well-defined** — it wraps modulo 2^N. `UINT_MAX + 1`
is always 0. This makes unsigned types safer for values that might
accidentally overflow (like adding user input), but it also means bugs can
hide longer because the wrap is silent.

**Guideline:** Use unsigned for values that are conceptually non-negative
and where negative results indicate a bug (HP, gold, counts). Use signed
for values that can legitimately go negative (damage deltas, coordinate
offsets) and for loop indices where you might count backwards.

### `sizeof` and measuring types

The **`sizeof`** operator reports the size of a type or variable in bytes,
evaluated at compile time:

```c
printf("int: %zu bytes\n", sizeof(int));
printf("unsigned: %zu bytes\n", sizeof(unsigned));
printf("char: %zu bytes\n", sizeof(char));              /* always 1 */
printf("float: %zu bytes\n", sizeof(float));

char buffer[64];
printf("buffer: %zu bytes\n", sizeof buffer);           /* 64 */
```

`%zu` is the format specifier for `size_t`, the type `sizeof` returns.

You'll use `sizeof` constantly: for sizing buffers, allocating memory
(Chapter 10), and understanding struct layout. **Practice
`05_sizeof_alignment`** makes this concrete.

### Casting: forcing a type conversion

An explicit **cast** tells the compiler to treat a value as a different
type for one expression. You will need this constantly for two reasons:
integer-to-float conversions to avoid truncation, and silencing alignment
or signedness warnings when you know better than the compiler.

#### The integer division trap

Here's a five-line trap that catches nearly everyone once:

```c
unsigned current_hp = 15;
unsigned max_hp = 20;

unsigned percent = current_hp / max_hp * 100;
printf("%u%%\n", percent);   /* prints 0%, not 75% */
```

`current_hp / max_hp` is `15 / 20`. Both operands are unsigned integers, so
C performs **integer division**: it computes the mathematically exact answer
(0.75) and then throws away everything after the decimal point, giving `0`.
Then `0 * 100` is still `0`. The percentage you wanted — 75% — never had a
chance to exist; it was destroyed before the multiplication even ran.

The fix is a cast to `float`:

```c
float percent = (float)current_hp / max_hp * 100;
printf("%.0f%%\n", percent);   /* prints 75% */
```

`(float)current_hp` converts just that one value to a `float` *before* the
division. Once one side of `/` is a `float`, C promotes the other side to
match, so the division is now real (fractional) division. Try both versions:

```c
#include <stdio.h>

int main(void)
{
    unsigned current_hp = 15;
    unsigned max_hp = 20;

    unsigned bad_percent = current_hp / max_hp * 100;
    float good_percent = (float)current_hp / max_hp * 100;

    printf("Integer division:  %u%%\n", bad_percent);
    printf("With a cast:       %.0f%%\n", good_percent);

    return 0;
}
```

```
Integer division:  0%
With a cast:       75%
```

Compile and run this short program yourself to see the difference. Save it
as `percent_test.c`, compile with `gcc -std=c17 -Wall -Wextra -g -o
percent_test percent_test.c`, and run `./percent_test`.

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

`unsigned hp = -5;` (where -5 is implicitly converted) gives you 4294967291,
not a compile error. The type system will not save you from logic bugs.
Clamping HP to `[0, max_hp]` is **game logic**, not a type guarantee —
**practice `04_clamp_hp`** before you expect battle code to do it later.


## Apply it

Pick up `main.c` where Chapter 1 left it. Replace the last two `printf`
calls (the placeholder-title note) with a name prompt and a character
sheet:

```c
char name[64];
printf("What is your name, hero? ");
fgets(name, sizeof name, stdin);

unsigned hp = 20;
unsigned mp = 8;
unsigned attack = 6;
unsigned defense = 4;
char rank = 'E';
float power_rating = (float)(attack + defense) / 2.0f;

printf("\n");
printf("Welcome, %s", name);
printf("----------------------------------------\n");
printf("HP:     %u\n", hp);
printf("MP:     %u\n", mp);
printf("ATK:    %u\n", attack);
printf("DEF:    %u\n", defense);
printf("Rank:   %c\n", rank);
printf("Power:  %.1f\n", power_rating);
printf("----------------------------------------\n");
```

**Why `unsigned` for these stats?** HP, MP, and the primary combat stats
are conceptually non-negative. A negative attack value is not a valid game
state — it is a bug. Using `unsigned` documents that intent and makes the
bug (if it happens) far more visible: subtracting 1 from 0 gives you
4,294,967,295, which will be noticed immediately, rather than -1, which
might slip through undetected. This is defensive design.

The starting values (`20`/`8`/`6`/`4`/`'E'`) are placeholders — real
balance numbers are far off and not worth agonizing over yet. `power_rating`
exists specifically to give you a harmless place to practice the cast from
the spotlight section on real game data.

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
would report 4 bytes for each `unsigned`, 1 for the `char`, 4 for the
`float`, 64 for `name`'s buffer — and each `printf` format specifier (`%u`
for unsigned, `%c` for char, `%f` for float, `%s` for string) told `printf`
how to interpret the bits sitting in that memory as something printable.

That reinterpretation is exactly why format specifiers have to match their
arguments. `printf` trusts you completely about what type is coming and has
no way to verify it at runtime. `-Wformat` (bundled into `-Wall`) *can*
check at compile time, most of the time — which is exactly why you never
turn these warnings off. See the first "Common error" below for what happens
when that check is the only thing standing between you and a crash.

Notice: `sizeof(unsigned)` and `sizeof(int)` are the same (4 bytes on this
platform). Choosing `unsigned` over `int` is not about saving space — it is
about **documenting intent** and making certain classes of bugs (accidental
negative values) visible immediately rather than silent.

## Common errors

### A mismatched format specifier — caught by the compiler:

If you change
`printf("HP:     %u\n", hp);`
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
address; go read a string starting there." The value `20` isn't a
valid address at all, so `printf` crashed trying to read from it. This is
**undefined behaviour**: the C standard puts no requirement on what happens
when you do this, so *anything* could result — a crash (what you saw here),
silently wrong output, or apparent success that fails later on a different
machine or compiler. The compiler's warning was your only warning; heed it.

(Note: The exact type in the warning will match whatever type `hp` has in
your code — `unsigned int` if you followed the Apply It section, `int` if
you are experimenting with variations.)

### Using `%d` (signed) for an `unsigned` value — subtle and usually silent:

```c
unsigned hp = 20;
printf("HP: %d\n", hp);   /* %d expects int, gets unsigned */
```

This compiles with a warning and *usually* prints correctly for small
values, but if `hp` ever holds a value larger than `INT_MAX` (2,147,483,647),
`%d` will reinterpret the bits as a signed value and print a negative
number. Always use `%u` for `unsigned`, `%d` for `int`. The compiler will
warn you if you mix them — listen.

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

1. *Practice.* Finish `05_sizeof_alignment`, format specs, int division,
   casted average, HP clamp.
2. *Wraparound.* Assign `hp = 0; hp = hp - 1;` — what value does `hp` hold?
   Why is this a bug rather than undefined behaviour? (Restore afterward.)
3. *Precedence.* Remove the `(float)` cast from `power_rating` and change
   the formula to `attack + defense / 2`. Is the bug integer division,
   precedence, or both?
4. *Type choice.* Should a loop counter `for (int i = 0; ...)` use `int` or
   `unsigned`? Should an array of 10,000 HP values use `unsigned` or
   `uint16_t`? Explain your reasoning for each.

<details>
<summary>Solutions</summary>

1. Practice `solutions/`.
2. `hp` becomes 4,294,967,295 (`UINT_MAX`). Unsigned wraparound is
   well-defined but almost always a logic bug — validation needed.
3. Both: `/` binds tighter than `+` (so it's `attack + (defense/2)`), and
   integer division truncates, so 6+4 gives 8, not 5.0. Needs `(float)(attack
   + defense) / 2.0f` or similar.
4. Loop counter: `int`. Promotion, signed math for `i--`, compiler expects
   it. Array: `uint16_t` if HP caps below 65535 and you have thousands —
   real savings. Single local: `unsigned` is fine, documents intent, no
   performance cost either way.

</details>

## Next up

Right now your "game" runs top to bottom once and stops. Chapter 3 gives it
a pulse: `if`, loops, and functions, building toward your first actual
interaction — a one-round coin-flip duel with a real winner and loser.

---

### A note on type choices as the game grows

This chapter uses `unsigned` for HP and stats to teach the distinction
between signed and unsigned, and to make the case that values that should
never be negative benefit from types that document that constraint. Later
chapters may keep `int` for some values despite that principle, and when
they do, they will explain why: library interop, avoiding a cascade of casts,
or because the value participates in signed arithmetic (damage deltas,
coordinate offsets). The choice is always a trade-off, never automatic. What
matters is that you understand the costs and document the decision.
