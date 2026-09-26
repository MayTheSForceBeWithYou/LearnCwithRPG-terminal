# Appendix C: Debugging Reference

A working reference for the four tools this course uses: `gdb`,
AddressSanitizer, UndefinedBehaviorSanitizer, and Valgrind's memcheck.

Introduced properly in Chapter 10 (`gdb`, ASan) and Chapter 22 (Valgrind,
UBSan). This is the version you keep open while debugging.

---


## Related practice

- Heap leaks / ownership drills: `code/ch10/practice/`
- Tool coverage (why tests miss paths): `code/ch22/practice/01-tool-coverage/`
- Assertions vs tests: `code/ch22/practice/02-assertions-vs-tests/`

## Which tool for which problem

None of these overlaps completely with the others. That is the single most
important thing on this page.

| | ASan | UBSan | Valgrind | gdb |
|---|---|---|---|---|
| Out-of-bounds read/write | **yes** | no | yes | no |
| Use after free, double free | **yes** | no | yes | no |
| Leaks | yes | no | **yes, in detail** | no |
| **Uninitialised reads** | **no** | no | **yes** | no |
| Signed overflow, bad shifts | no | **yes** | no | no |
| Where did it crash, and why | no | no | partly | **yes** |
| Speed | ~2× slower | ~1.2× | ~30× slower | normal |
| Needs recompiling | yes | yes | **no** | `-g` only |

The row that catches people is **uninitialised reads**. ASan does not look
for them — it tracks where memory *is*, not whether it has a value. In
Chapter 22 that gap hid twenty real errors through seven chapters of clean
ASan runs.

**Run ASan+UBSan routinely; run Valgrind before you believe you are done.**

---

## gdb

### Getting a usable build

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -O0 -o game *.c -lncursesw
```

`-g` adds debug information; `-O0` stops the optimiser from inlining
functions and discarding variables you want to look at. Debugging an `-O2`
build is possible but you will see `<optimized out>` a lot.

### The five commands that matter

```
run          (r)   start the program
break FUNC   (b)   stop when FUNC is entered  -- also break file.c:42
backtrace    (bt)  how did I get here
print EXPR   (p)   show a variable or expression
continue     (c)   carry on
```

Nine times out of ten the session is: `run`, crash, `bt`, `print`.

### A worked example

```c
static int divide(int a, int b) { return a / b; }
static int middle(int n) { return divide(100, n - 3); }
int main(void) { int *p = malloc(sizeof *p); *p = 3;
                 printf("%d\n", middle(*p)); free(p); return 0; }
```

```bash
gdb ./crash
```

```
(gdb) run
Program received signal SIGFPE, Arithmetic exception.
0x0000555555555167 in divide (a=100, b=0) at crash.c:3
3	static int divide(int a, int b) { return a / b; }
```

It already told you a great deal: the signal (`SIGFPE`, integer division by
zero), the function, the line, and the argument values — `b=0`.

```
(gdb) bt
#0  0x0000555555555167 in divide (a=100, b=0) at crash.c:3
#1  0x0000555555555189 in middle (n=3) at crash.c:4
#2  0x00005555555551b8 in main () at crash.c:5
```

Read a backtrace **bottom to top**: `main` called `middle(3)`, which called
`divide(100, 0)`. Frame `#1` is where the bad value was computed, and frame
`#0` is merely where it went off. *The crash site is rarely the bug site.*

`print` works in the currently selected frame, which starts at `#0`:

```
(gdb) print n
No symbol "n" in current context.
```

Select the frame that has it:

```
(gdb) frame 1
#1  0x0000555555555189 in middle (n=3) at crash.c:4
4	static int middle(int n) { return divide(100, n - 3); }
(gdb) print n
$1 = 3
```

There it is: `n` is 3, so `n - 3` is 0.

### Breakpoints

```
(gdb) break divide
(gdb) run
Breakpoint 1, divide (a=100, b=0) at crash.c:3
(gdb) info args
a = 100
b = 0
```

| | |
|---|---|
| `break file.c:42` | stop at a line |
| `break func if x > 5` | stop only when a condition holds |
| `info args` / `info locals` | everything in scope |
| `next` (`n`) | run one line, stepping *over* calls |
| `step` (`s`) | run one line, stepping *into* calls |
| `finish` | run until the current function returns |
| `delete` | remove all breakpoints |

### Inspecting your data

```
(gdb) print *player            /* the whole struct */
(gdb) print player->hp
(gdb) print battle->enemies[0]
(gdb) print battle->enemies[0].type->name
(gdb) print/x flags            /* in hex -- good for bit flags */
(gdb) print *map->tiles@20     /* 20 elements from a pointer */
```

That last one is the trick worth remembering: `@count` prints an array when
all you have is a pointer, which is most of the time in C.

### Debugging an ncurses program

`gdb` and ncurses both want the terminal, and the result is unreadable. Run
the game in one terminal and attach from another:

```bash
# terminal 1
./game
# terminal 2
gdb -p $(pgrep -n game)
```

Or let it crash and inspect the corpse:

```bash
ulimit -c unlimited          # allow core dumps in this shell
./game                       # crash
gdb ./game core              # or: coredumpctl gdb game
```

### Batch mode

Useful in a script, or when you just want the backtrace:

```bash
gdb -q -batch -ex run -ex bt ./game
```

---

## AddressSanitizer and UBSan

### Building

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined -o game *.c \
    -lncursesw -fsanitize=address,undefined
```

The flag is needed at **both** compile and link time — it is a library as
well as an instrumentation pass. In this project, `make sanitize` does it.

Keep `-g`: without it the reports have addresses instead of line numbers.

### Reading an ASan report

```
==302216==ERROR: AddressSanitizer: heap-use-after-free on address 0x77c4bb4e0010
READ of size 4 at 0x77c4bb4e0010 thread T0
```

Three things, in order of usefulness:

1. **The kind of error** — `heap-use-after-free`, `heap-buffer-overflow`,
   `stack-buffer-overflow`, `double-free`.
2. **The stack where it happened**, top frame first.
3. **Two more stacks below it**: where the block was *allocated*, and (for
   use-after-free) where it was *freed*. That second pair is usually what
   solves it.

Leaks appear at exit:

```
Direct leak of 100 byte(s) in 1 object(s) allocated from:
SUMMARY: AddressSanitizer: 100 byte(s) leaked in 1 allocation(s).
```

"Direct" means nothing points at it; "indirect" means it was only reachable
through something already leaked. Fix the direct ones and the indirect ones
usually vanish with them.

### UBSan

UBSan reports one line per event and, by default, keeps going:

```
party.c:185:11: runtime error: signed integer overflow: 100 + 2147483637
cannot be represented in type 'int'
```

It catches signed overflow, shifts past the width of a type, misaligned or
null pointer use, and array indices it can prove are out of range.

To stop at the first one and get a backtrace:

```bash
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./game
```

### Useful environment options

```bash
ASAN_OPTIONS=detect_leaks=0 ./game          # suppress leak reporting
ASAN_OPTIONS=abort_on_error=1 ./game        # dump core at the first error
ASAN_OPTIONS=log_path=/tmp/asan ./game      # write reports to files
```

`detect_leaks=0` is genuinely useful for a program with a known,
uninteresting leak in a third-party library, so the real errors stay
visible.

---

## Valgrind (memcheck)

### Running

No recompilation needed — but keep `-g` so reports carry line numbers.

```bash
valgrind --leak-check=full --track-origins=yes ./game
```

| flag | why |
|---|---|
| `--leak-check=full` | list each leak with its allocation stack |
| `--track-origins=yes` | for uninitialised values, say where they came from |
| `--suppressions=FILE` | ignore known-innocent reports |
| `--error-exitcode=9` | non-zero exit on any error, for CI |
| `-s` | show which suppressions fired |

`--track-origins=yes` is what turns an unactionable report into a fixable
one. Without it you learn where a bad value was *used*, which is rarely
where it came from.

### On Arch: it refuses to start

```
valgrind: Fatal error at startup: a function redirection
valgrind: which is mandatory for this platform-tool combination
valgrind: cannot be set up.
```

Arch ships a stripped `ld.so`, and the packages Valgrind's message
recommends (`libc6-dbg`, `glibc-debuginfo`) do not exist here. The fix is
debuginfod — see [Appendix A](a-wsl-troubleshooting.md).

```bash
echo $DEBUGINFOD_URLS      # empty? open a new shell, or:
DEBUGINFOD_URLS=https://debuginfod.archlinux.org valgrind ./run_tests
```

### Reading an uninitialised-value report

```
Conditional jump or move depends on uninitialised value(s)
   at 0x400E45D: party_apply_level (party.c:171)
   by 0x4003DCC: test_xp_and_levelling (test_combat.c:258)
   by 0x400DA19: main (test_combat.c:1511)
 Uninitialised value was created by a stack allocation
   at 0x4003D46: test_xp_and_levelling (test_combat.c:249)
```

Read the **bottom block first**: that is where the bad value was born
(`test_combat.c:249`). The top block is only where it was finally noticed.
Fix the origin.

### The four leak categories

- **definitely lost** — nothing points at it. A real leak. This is the
  number that matters.
- **indirectly lost** — only reachable via something definitely lost.
- **possibly lost** — a pointer into the middle of the block. Usually a
  library storing an interior pointer.
- **still reachable** — a live pointer exists at exit. Not a leak in any
  useful sense.

### Suppressions

Noise you cannot fix is worse than useless: it trains you to skim a report
you should read. ncurses keeps global terminal state for the life of the
process, and `endwin()` does not release all of it, by design.

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
leak whose stack passes through `initscr`".

Write them **narrowly** — name the specific library function — and never
suppress `definitely lost`. A suppression hides real bugs exactly as well
as fake ones.

Generate a starting point with `--gen-suppressions=all`, then cut it down.

---

## Driving an ncurses program from a script

Worth knowing, because the obvious approach silently produces fake results.

```bash
printf 'Wick\n\nwasdq' | ./game        # does NOT work
```

`fgets` is buffered: it reads as much as the pipe offers into a `FILE`
buffer, swallowing the whole input. `getch()` then reads file descriptor 0
directly, finds nothing, and returns `ERR` forever — the game spins at full
speed. Worse, a run killed by `timeout` **still prints a Valgrind leak
summary**, so you get numbers that look real but come from a program that
never reached its cleanup.

Use a pseudo-terminal instead. See Chapter 22 for a complete script; the
essentials are `pty.openpty()`, writing keys to the master end, and
*draining the output* so the buffer cannot fill and deadlock `refresh()`.

---

## A debugging order that works

1. **Read the error.** Compilers and sanitizers usually say exactly what is
   wrong. This step is skipped more often than any other.
2. **Turn up the compiler.** `-Wall -Wextra -Wpedantic`, then the stricter
   set from Chapter 22. Free, instant, no false crashes.
3. **Reproduce it reliably.** An intermittent bug is a bug you cannot
   confirm you fixed. Seed the RNG; write the failing case as a test.
4. **Run ASan+UBSan.** Cheap, and catches most memory errors immediately.
5. **Run Valgrind.** Slower, and the only one that sees uninitialised reads.
6. **Then reach for gdb**, to answer "how did it get into this state".
7. **Write a test** that would have caught it, before fixing it.

Step 7 is the one that compounds. Chapter 15's test harness exists because
of it, and by Chapter 24 it runs three hundred thousand assertions in a
second.
