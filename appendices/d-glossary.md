# Appendix D: Glossary

Every term this course defines on first use, collected in one place, with
the chapter that introduces it properly.

Entries are written to be read cold — if you have forgotten what a
translation unit is in Chapter 19, this should be enough without going back.

---

### AddressSanitizer (ASan)
*Chapter 10.* A compiler instrumentation pass (`-fsanitize=address`) that
checks memory access at run time: out-of-bounds, use-after-free,
double-free, leaks. Roughly doubles run time. **Does not detect reads of
uninitialised memory** — that needs Valgrind. See
[Appendix C](c-gdb-and-sanitizers.md).

### argument
*Chapter 3.* The value actually passed at a call site. The variable that
receives it inside the function is the **parameter**. `f(3)` passes the
argument `3`.

### array decay
*Chapter 7.* An array name used in most expressions converts automatically
to a pointer to its first element. This is why `sizeof arr` gives the array
size inside the function that declared it, but only the pointer size in a
function it was passed to — the length did not travel with it.

### assertion
*Chapter 22.* A runtime claim that must be true, which aborts the program if
it is not. `assert(x > 0)`. For conditions that **cannot happen unless the
code is wrong** — not for error handling. Compiled out entirely when
`NDEBUG` is defined, so it must never contain side effects.

### bit flags
*Chapter 18.* Independent true/false facts packed into the bits of one
integer, so several can be true at once. `status & STATUS_ASLEEP` tests one;
`status |= STATUS_ASLEEP` sets it; `status &= ~STATUS_ASLEEP` clears it.

### buffer overrun
*Chapter 12.* Writing past the end of an array. Undefined behaviour, and the
classic C security hole. `snprintf` and `fgets` take a size for this reason.

### calling convention
*Chapter 24.* The agreed rules for how arguments and return values are
passed between functions — which registers, what order, who cleans up. You
never write it, but it is why a `va_list` can work at all.

### const-correctness
*Chapter 7.* Marking pointers `const` when the function does not modify what
they point at: `int entity_attack(const Player *p)`. Documents intent and
lets the compiler enforce it.

### dangling pointer
*Chapter 10.* A pointer to memory that has been freed or has gone out of
scope. Using one is undefined behaviour; it frequently appears to work.

### declaration / definition
*Chapter 4.* A **declaration** says a thing exists and what its type is
(`int map_width(void);`). A **definition** provides the actual body or
storage. Headers declare; `.c` files define. "Undefined reference" means the
linker found a declaration with no definition.

### dispatch table
*Chapter 14.* An array of function pointers indexed by a value, replacing a
long `switch`. This game's modes are dispatched this way, so adding a mode
means adding a table row rather than editing a `switch`.

### dynamic linking
*Chapter 24.* Resolving library references when the program *starts* rather
than when it is built. The binary records a dependency
(`libncursesw.so.6`) which the dynamic loader satisfies at run time. Smaller
binaries and shared security fixes; requires the library to be present. See
also **static linking**.

### endianness
*Chapter 20.* The order in which a multi-byte number is stored — most
significant byte first (big-endian) or last (little-endian). Matters the
moment you write raw bytes to a file and read them on another machine. This
project's text save format sidesteps it deliberately.

### file descriptor
*Chapter 22.* A small integer identifying an open file to the kernel. 0 is
standard input. Distinct from a `FILE *`, which is a buffered layer *on top*
of one — which is why mixing `fgets` and `getch()` on the same descriptor
goes wrong.

### format string
*Chapter 2.* The first argument to `printf` and friends, containing
conversion specifiers (`%d`, `%s`). Because variadic arguments carry no type
information at run time, **the format string is the only record of what was
passed**; a mismatch is undefined behaviour.

### function pointer
*Chapter 14.* A variable holding the address of a function, so which code
runs can be decided at run time. `typedef void (*SpellEffect)(const Spell *,
SpellContext *);`

### fuzzing
*Chapter 22.* Feeding a program randomly generated input to see whether it
survives. Cheapest way to test a parser: this project throws 4,000 random
files at all five of its parsers with `make fuzz`.

### header guard / include guard
*Chapter 4.* The `#ifndef FOO_H / #define FOO_H / #endif` wrapper that stops
a header being processed twice in one translation unit, which would
redefine its types.

### heap
*Chapter 10.* Memory obtained with `malloc` and released with `free`. Its
lifetime is whatever you choose, which is the power and the entire problem.
Contrast **stack**.

### integer promotion
*Chapter 15.* Small integer types are converted to `int` before arithmetic.
Also why `float` arguments to variadic functions become `double`, and why
`printf("%f")` handles both.

### leak
*Chapter 10.* Allocated memory that is never freed and can no longer be
reached. Harmless in a program that exits at once, fatal in one that runs
for hours.

### linker
*Chapter 4.* The program that combines object files and libraries into an
executable, resolving each symbol reference to a definition. Its
characteristic complaint is `undefined reference to 'foo'`.

### lvalue
*Chapter 2.* Something that can appear on the left of an assignment — it
designates storage. `x = 5` is fine; `5 = x` gives *lvalue required as left
operand of assignment*.

### `malloc` / `free`
*Chapter 10.* Request and release heap memory. `malloc` returns `NULL` on
failure and **must be checked, every time**. Every allocation needs a
documented owner; this project pairs `*_create` with `*_destroy`.

### NUL terminator
*Chapter 12.* The zero byte that ends a C string. Not the same as the
character `'0'`. Every `str*` function depends on it; lose it and `strlen`
walks off into whatever is next.

### object file
*Chapter 4.* The `.o` produced by compiling one `.c` — machine code with
unresolved references to anything defined elsewhere.

### opaque interface
*Chapter 8.* A header exposing functions but not the structure behind them,
so callers cannot depend on the implementation. `render.h` is one: the game
draws without knowing whether it is talking to a terminal or a window.

### out-parameter
*Chapter 7.* A pointer argument a function writes its result through, used
when a function must return more than one thing:
`int paths_save(char *out, size_t out_size)`.

### parameter
See **argument**.

### pass-by-value / pass-by-reference
*Chapters 6 and 7.* C always passes by value — the function gets a copy.
`move_player(Player p)` modifies a copy and the caller sees nothing.
Pass-by-reference is achieved by passing a *pointer* to the thing, which is
the failure Chapter 6 stages deliberately so Chapter 7 can explain it.

### pointer
*Chapter 7.* A variable holding a memory address. `&x` takes the address of
`x`; `*p` reads or writes what `p` points at. `NULL` points at nothing and
must never be dereferenced.

### pointer arithmetic
*Chapter 7.* Adding to a pointer moves it by *elements*, not bytes: for
`int *p`, `p + 1` advances four bytes on a typical machine. `arr[i]` is
defined as `*(arr + i)`.

### preprocessor
*Chapter 4.* The first stage of compilation. It handles `#include`,
`#define` and `#ifdef` as pure text substitution, before the compiler sees
any C. `-DNAME=VALUE` defines a macro from the command line.

### pseudo-terminal (pty)
*Chapter 22.* A kernel object that behaves like a terminal on one end and a
pipe on the other. Necessary to drive an ncurses program from a script,
because a plain pipe is not a terminal and stdio buffering swallows the
input.

### pure function
*Chapter 15.* A function whose result depends only on its arguments and
which changes nothing outside itself. `combat_base_damage` is one, which is
why it can be exhaustively tested without setting up a game.

### `realloc`
*Chapter 17.* Resizes an existing allocation, possibly moving it. **Never
assign it directly to the pointer you passed in** — if it fails it returns
`NULL` and you have lost the original. Assign to a temporary, check, then
commit.

### segmentation fault
*Chapter 7.* The kernel killing your process for touching memory it does not
own. A symptom, not a diagnosis: the cause is usually a null, dangling, or
wildly out-of-range pointer.

### serialisation
*Chapter 20.* Turning in-memory state into bytes that can be stored, and
back again. The reverse direction must validate everything, because a file
on disk is untrusted input even when your own program wrote it.

### stack
*Chapter 10.* Memory for local variables and call frames, allocated and
released automatically as functions enter and return. Fast, limited in size,
and the reason returning a pointer to a local is a bug. Contrast **heap**.

### static
*Chapter 4.* Two unrelated meanings. On a file-scope function or variable it
means *internal linkage* — invisible outside its own translation unit. On a
local variable it means *lasts for the whole program*, which is the sense
that surprises people.

### static linking
*Chapter 24.* Copying library code into the executable at build time,
producing a self-contained but larger binary that does not benefit from
library updates. Contrast **dynamic linking**.

### string literal
*Chapter 12.* A quoted constant like `"Wick"`. Stored in read-only memory:
writing through a `char *` that points at one is undefined behaviour, so
declare such pointers `const char *`.

### struct padding
*Chapter 20.* Unused bytes the compiler inserts between struct members so
each is properly aligned. It means `sizeof(struct)` is not the sum of its
members, and it is one reason writing a struct's raw bytes to a file is a
trap.

### tokenising
*Chapter 21.* Splitting an input line into meaningful pieces before
interpreting them. This project uses `sscanf` with width-limited
conversions (`%15s`) to read its data tables, and checks the returned field
count rather than assuming the line was well formed.

### translation unit
*Chapter 4.* One `.c` file *after* preprocessing — the source plus
everything it included. The unit the compiler actually works on, and the
scope `static` restricts things to.

### UndefinedBehaviorSanitizer (UBSan)
*Chapter 22.* `-fsanitize=undefined`. Reports signed overflow, bad shifts,
misaligned pointers and similar at run time, one line per event.

### undefined behaviour (UB)
*Chapter 10.* Code for which the standard imposes **no requirement
whatsoever**. Not "unpredictable output" — the compiler may assume it never
happens and optimise accordingly. This is why "it ran fine" proves nothing,
and why code can break on a compiler upgrade.

### Valgrind / memcheck
*Chapter 22.* A tool that runs your program in a simulator, tracking the
definedness of every bit. ~30× slower than native and needs no
recompilation. **The only tool here that finds uninitialised reads.**

### variadic function
*Chapter 23.* A function taking a variable number of arguments, declared
with `...` and read with `<stdarg.h>`. Type-unsafe by nature: nothing
records what was passed, which is why `printf` needs a format string.

### warning
*Chapter 1.* The compiler telling you something is probably wrong while
compiling it anyway. This course treats warnings as errors in spirit: every
snapshot builds clean under `-Wall -Wextra -Wpedantic`.

### word wrap
*Chapter 12.* Breaking text into lines at spaces so words are not split.
The dialogue system pages as well as wraps, so long speeches are shown in
full rather than truncated.

### XDG Base Directory specification
*Chapter 24.* The Linux convention for where user files belong.
`$XDG_DATA_HOME` (default `~/.local/share`) holds user data — which is where
this game's save goes, rather than next to the binary.
