# 02 — Assertions vs tests

When a floor invariant breaks, an `assert` and a table-driven `CHECK` tell
you different stories. The assert names the **line inside the function** that
computed a bad value. The test names **inputs and the wrong answer** and can
keep going so you see three failures, not one.

Chapter 22's old exercise deleted the production damage floor to compare
messages. Use `-DDEMO_BAD_FLOOR` / `make demo-assert` here instead.

## What this lesson asks of you

Read the complementary roles. Reject deleting the production floor. Then open
`TASK.md`, add `assert(base >= 1)`, keep the lasting floor in the default
build, and compare `make check` vs `make demo-assert`.

## Foundations — both, not either

```c
int base = atk + level / 2 - defense;
if (base < 1) base = 1;   /* lasting floor — production keeps this */
assert(base >= 1);        /* documents the invariant at the site */
```

| Build | Floor present? | What you see on a bad case |
| ----- | -------------- | -------------------------- |
| default (`make check`) | Yes | Checks pass; assert is insurance |
| `make demo-assert` (`-DDEMO_BAD_FLOOR`) | No | Assert aborts *or* checks FAIL with inputs |

`abort()` may not flush stdout — worth knowing at 2am.

## Worked example

**Step 1 —** Default build: heavy defence floors to 1; checks pass.

**Step 2 —** Demo build: floor compiled out; assert (once added) fires at the
function line; without assert, harness prints `FAIL` with inputs.

**Step 3 — rejected wrong reading.** "Temporarily delete the floor in
`combat_math.c` and run the real tests." Same lesson, wrong tree. The demo
flag is the reading exhibit; production keeps the floor permanently.

## Check yourself

1. What does the assert show that the test does not?
2. What does the test show that the assert does not?
3. Why is "assertions win, delete the tests" the wrong lasting habit?

## Key takeaways

- Asserts locate; tests describe inputs and can continue.
- Keep both.
- Drop the floor only via `DEMO_BAD_FLOOR` in this drill.

## Lookup

- Chapter 22 Exercises solutions (historical contrast)
- `NDEBUG` (next micro: `02_ndebug_toy`)

Now open `TASK.md`.
