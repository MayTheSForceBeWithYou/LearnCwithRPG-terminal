# Appendix B: C Pitfalls

The mistakes this course walked you into on purpose, plus the ones it
steered around, collected so you can look them up when something is wrong
and you do not yet know why.

Each entry: what it looks like, why C allows it, and what to do instead.
The chapter reference is where the course covers it properly.

---

## How to use this appendix

Start from the **symptom**, not the cause — that is the order you meet them
in.

| Symptom | Look at |
|---|---|
| Compiles, runs, wrong number | [Integer division](#integer-division-truncates), [Signed overflow](#signed-overflow-is-undefined), [Operator precedence](#precedence-surprises) |
| Compiles, crashes immediately | [NULL dereference](#dereferencing-null), [Array bounds](#walking-off-an-array) |
| Compiles, crashes *sometimes* | [Uninitialised reads](#reading-uninitialised-memory), [Use after free](#use-after-free), [Dangling pointers](#returning-a-pointer-to-a-local) |
| Works at `-O0`, breaks at `-O2` | Anything undefined. Start with [UB](#undefined-behaviour-in-general) |
| `undefined reference to ...` | [Linker errors](#undefined-reference) |
| Text is truncated or garbled | [NUL termination](#a-string-without-its-nul), [Buffer overruns](#buffer-overruns) |
| Memory grows forever | [Leaks](#leaks) |

---

## Undefined behaviour, in general

*Chapter 10, and named explicitly from there onward.*

**Undefined behaviour** means the standard imposes no requirement at all.
Not "returns garbage" — *no requirement*. The compiler may assume it never
happens and optimise on that assumption.

This is why "it ran fine" proves nothing, and why the same code can work for
a year and break when you add `-O2` or upgrade gcc. The compiler did not
become hostile; it started using an assumption you had been violating all
along.

The practical consequence, and the reason Chapter 22 exists: **you cannot
test for UB by running the program.** You need tools that look for it.

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined ...
valgrind --leak-check=full --track-origins=yes ./program
```

---

## Reading uninitialised memory

*Chapters 17 and 22.*

```c
Player p;                 /* nothing written */
party_apply_level(&p);    /* reads p.hp -- undefined behaviour */
```

A local variable starts with whatever bytes were on the stack. Reading it
before writing it is UB, not merely untidy.

```
warning: ‘x’ is used uninitialized [-Wuninitialized]
```

...but only when the compiler can see it, which is often not the case
across function calls. **AddressSanitizer does not catch this** — it tracks
where memory is, not whether it has a value. Valgrind's memcheck does:

```
Conditional jump or move depends on uninitialised value(s)
   at 0x400E45D: party_apply_level (party.c:171)
 Uninitialised value was created by a stack allocation
   at 0x4003D46: test_xp_and_levelling (test_combat.c:249)
```

**Fix:** initialise at declaration, or `memset` the struct before anything
reads it. And beware the trap in the next entry.

---

## `memset` does not mean "empty"

*Chapters 22 and 23.*

```c
memset(battle, 0, sizeof *battle);
/* battle->boss_id is now 0 -- which is BOSS_BOGWRIGHT */
```

Zeroing sets every field to zero, and zero is a *value*. If the first
member of an enum is `BATTLE_ONGOING` or `BOSS_BOGWRIGHT`, a zeroed struct
makes a confident false claim. No warning, no crash.

**Fix:** when zero is a legal value of a field, say "absent" explicitly:

```c
memset(battle, 0, sizeof *battle);
battle->boss_id = -1;
```

Or design the enum so its zero member means "none":
`BOSS_NONE = 0, BOSS_BOGWRIGHT, ...`.

---

## Use after free

*Chapter 10.*

```c
free(map);
printf("%d\n", map->width);     /* the pointer still points somewhere */
```

`free` does not change your pointer; it hands the memory back to the
allocator. Reading it often works, right up until the allocator reuses it.

```
==302216==ERROR: AddressSanitizer: heap-use-after-free on address 0x77c4bb4e0010
READ of size 4 at 0x77c4bb4e0010 thread T0
```

**Fix:** set the pointer to `NULL` after freeing, and give every allocation
a documented owner — the `*_create` / `*_destroy` pairing this course uses
everywhere.

---

## Double free

*Chapter 10.*

Freeing the same block twice corrupts the allocator's bookkeeping. It
frequently crashes somewhere *else*, later, in code that is fine.

**Fix:** one owner per allocation. `free(NULL)` is explicitly legal and does
nothing, so `NULL`-ing after free makes a second free harmless.

---

## Leaks

*Chapter 10.*

```
Direct leak of 100 byte(s) in 1 object(s) allocated from:
SUMMARY: AddressSanitizer: 100 byte(s) leaked in 1 allocation(s).
```

Not UB, and harmless in a program that exits immediately — which is exactly
why they survive: nothing goes wrong until the program runs for hours.

Valgrind splits them four ways, and only the first is unambiguously a bug:

- **definitely lost** — nothing points at it. A real leak.
- **indirectly lost** — only reachable through something definitely lost.
- **possibly lost** — a pointer into the middle of the block, not its start.
- **still reachable** — a live pointer exists at exit. Usually fine.

---

## Returning a pointer to a local

*Chapter 7.*

```c
const char *name(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "Wick");
    return buf;              /* buf stops existing when we return */
}
```

```
warning: function returns address of local variable [-Wreturn-local-addr]
```

**Fix:** have the caller supply the buffer — `void name(char *out, size_t
n)` — which is the pattern `paths_data` and `dialog_wrap` use. A `static`
buffer also "works" and creates a subtler bug: two calls in one expression
and the first result is silently clobbered.

---

## Dereferencing NULL

*Chapter 7.*

`malloc` and `fopen` return `NULL` on failure. Using the result without
checking turns a recoverable failure into a crash — or worse, on some
paths, into UB the optimiser reasons about.

**Fix:** check every allocating call, every time. This course models it from
the first `malloc` onward, without exception.

---

## Walking off an array

*Chapter 5.*

C does not check bounds. `a[4]` in a four-element array reads whatever is
next in memory.

```
warning: iteration 4 invokes undefined behavior [-Waggressive-loop-optimizations]
```

That warning appears only when the compiler can prove it. In the general
case, ASan catches it at run time.

**Fix:** `for (int i = 0; i < n; i++)` — `<`, never `<=`. Derive the count
with `sizeof arr / sizeof arr[0]` rather than writing a literal that can
drift from the array.

---

## Buffer overruns

*Chapter 12.*

```c
char small[8];
strcpy(small, "a much longer string");
```

```
warning: ‘__builtin_memcpy’ forming offset [8, 20] is out of the bounds
[0, 8] of object ‘small’ with type ‘char[8]’ [-Warray-bounds=]
```

`strcpy` copies until the source's NUL. It has no idea how big the
destination is.

**Fix:** `snprintf(small, sizeof small, "%s", src)`. It always terminates
and it truncates rather than overflowing. Note `sizeof small`, not a
literal `8` — the literal is a bug waiting for someone to resize the array.

`strncpy` is **not** the safe version of `strcpy`: it does not terminate if
the source fills the buffer exactly.

---

## A string without its NUL

*Chapter 12.*

A C string is bytes followed by a zero byte. Lose the terminator and
`strlen` keeps walking into whatever is next, and `printf("%s")` prints it.

**Fix:** prefer `snprintf` and `fgets`, which always terminate. When you
build a string by hand, place the NUL deliberately and count it in your
buffer size.

---

## `fgets` keeps the newline

*Chapter 2.*

```c
fgets(name, sizeof name, stdin);   /* name is "Wick\n" */
```

This bites once and then never again — usually when the name later comes
from a save file instead of the keyboard and the formatting silently
changes.

**Fix:** trim it at the point of input, once, so every later use is a plain
string:

```c
size_t len = strlen(name);
while (len > 0 && (name[len - 1] == '\n' || name[len - 1] == '\r')) {
    name[--len] = '\0';
}
```

(`gets` does not exist any more. It was removed from the language in C11
because it could not be used safely — there was no way to tell it how big
the buffer was.)

---

## Integer division truncates

*Chapters 9 and 23.*

```c
int thirds = hp / max_hp * 3;      /* 0 until hp == max_hp, then 3 */
```

`hp / max_hp` is 0 for every value below full, and `0 * 3` is 0. The
fraction is discarded *before* the multiply.

**Fix:** multiply first: `(hp * 3) / max_hp`. Watch for overflow when the
numbers are large — the next entry.

---

## Signed overflow is undefined

*Chapters 15 and 22.*

```c
p->xp += xp;      /* undefined once the total passes INT_MAX */
```

Signed overflow is not a wrap to negative; it is UB. UBSan says so:

```
party.c:185:11: runtime error: signed integer overflow: 100 + 2147483637
cannot be represented in type 'int'
```

**Fix:** test *before* the addition, by rearranging so the check cannot
itself overflow:

```c
if (xp > PARTY_MAX_XP - p->xp) {    /* not: p->xp + xp > PARTY_MAX_XP */
    p->xp = PARTY_MAX_XP;
} else {
    p->xp += xp;
}
```

Unsigned overflow *is* defined — it wraps — which is why hashes and RNGs
use unsigned types deliberately.

---

## Precedence surprises

```c
if (flags & MASK == 0)        /* == binds tighter than & */
```

This is `flags & (MASK == 0)`, which is almost never what you meant.

**Fix:** parenthesise bitwise operations in comparisons:
`if ((flags & MASK) == 0)`.

---

## `=` where you meant `==`

```c
if (x = 5) { ... }        /* assigns, then tests 5, which is true */
```

With a constant on the left, the compiler saves you:

```
error: lvalue required as left operand of assignment
```

With a variable, it is legal C and `-Wall` gives only a warning. Turn
warnings into errors on your own code (`-Werror`) if you keep making this
one.

---

## `printf` format mismatches

*Chapters 2 and 23.*

```c
float f = 1.5f;
printf("%d\n", f);
```

```
warning: format ‘%d’ expects argument of type ‘int’, but argument 2 has
type ‘double’ [-Wformat=]
```

`printf` is variadic: the format string is the *only* record of what you
passed. Nothing checks it at run time, so a mismatch reads the wrong bytes.

Note the message says `double` for a `float` argument — variadic arguments
promote `float` to `double` automatically. That is why `%f` handles both.

**Fix:** heed `-Wformat`, and put `__attribute__((format(printf, n, m)))` on
your own variadic wrappers so they get the same checking.

---

## Undefined reference

*Chapter 4.*

```
p6.c:(.text+0x5): undefined reference to `f'
```

This is the **linker**, not the compiler. The declaration was visible (so
compiling succeeded) but no definition was found in any `.o` or library.

Usual causes:

- The `.c` file that defines it is missing from `SOURCES` in the `Makefile`.
- A library is missing from `LDLIBS` (`undefined reference to 'initscr'`
  means you left off `-lncursesw`).
- You declared it and never wrote it.
- The definition is `static`, so it is invisible outside its own file.

---

## Header included twice

*Chapter 4.*

Without an include guard, a header included through two paths defines its
types twice:

```
error: redefinition of ‘struct Player’
```

**Fix:** an include guard in every header, no exceptions:

```c
#ifndef PLAYER_H
#define PLAYER_H
/* ... */
#endif
```

---

## Side effects inside `assert`

*Chapter 22.*

```c
assert(inventory_add(inv, id, 1));
```

`assert` compiles to nothing when `NDEBUG` is defined — which is what
`make release` does. The item silently never arrives in the release build.

**Fix:** assert on values, never on actions.

---

## Comparing floats with `==`

```c
if (power_rating == 5.5f)     /* almost never reliable */
```

Most decimal fractions have no exact binary representation, so arithmetic
that "should" produce 5.5 may produce 5.499999. Compare against a tolerance,
or keep the value in integers — which is why every stat in this game is an
`int` and only the displayed power rating is a float.

---

## Modifying a string literal

```c
char *s = "Wick";
s[0] = 'N';           /* undefined behaviour; usually a segfault */
```

String literals live in read-only memory. The type is `char *` for
historical reasons, which makes this look allowed.

**Fix:** `const char *s = "Wick";` so the compiler objects, or
`char s[] = "Wick";` for a modifiable copy.
