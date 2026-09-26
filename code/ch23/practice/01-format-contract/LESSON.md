# 01 — Format attribute contract

A `printf`-style wrapper without `__attribute__((format(printf, m, n)))`
will happily compile a `%s`/int mismatch. At runtime, `vsnprintf` may treat
the int as a pointer — segfault, garbage, or (worst) nothing obvious. With
the attribute, gcc rejects the call at compile time.

Chapter 23's old exercise deleted the attribute on production `log_linef`.
Compare with/without **here** via `make bad` and `bad_call.c`; leave
production alone.

## What this lesson asks of you

Read the contract. Reject stripping production attributes. Then open
`TASK.md`, run the good path (`make check`), and study `make bad` outputs.

## Foundations — declare the contract next to the prototype

```c
static void log_linef(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
```

Argument 1 is the format string; argument 2 is the first vararg — same
numbering idea as `printf`. GCC/Clang use that to type-check call sites.

| Build | Attribute | Intentional `%s` + int |
| ----- | --------- | ---------------------- |
| `make` / `make check` | On (good calls) | Not present — runs clean |
| `make bad` without attr | Off | Often compiles (exit 0) |
| `make bad` with attr | On | `-Werror=format` fails the compile |

## Worked example

**Step 1 —** `main.c` logs `hero hp=%d name=%s` correctly.

**Step 2 —** `bad_call.c` passes `int x` to `%s`. `make bad` prints both
compiler transcripts.

**Step 3 — rejected wrong reading.** "Delete the attribute on real
`log_linef`, pass a bad call, see the runtime crash, then put it back." The
runtime surprise is real; leaving the attribute off (or forgetting restore)
is how format bugs return. The cheap bug is the one that fails at compile
time — keep the attribute on shipping wrappers.

## Check yourself

1. Why doesn't plain `-Wformat` always catch mismatches on *your* wrapper
   without the attribute?
2. What are three runtime shapes of a `%s`/int slip?
3. Where should the lasting attribute live after this drill?

## Key takeaways

- Format attributes move mismatches to compile time.
- `make bad` is the reading exhibit; production keeps the attribute.
- The invisible runtime miss is why you do not "try without it" in the game.

## Lookup

- Chapter 23 Common errors / `log_linef`
- GCC `format` attribute docs

Now open `TASK.md`.
