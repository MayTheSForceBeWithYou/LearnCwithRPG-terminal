# 01 — Tool coverage (what memcheck/ASan can see)

A sanitizer or `valgrind` memcheck can only report bugs on paths that
**execute**. "The test suite is clean" and "the program is clean" are
different claims when the suite never calls the buggy function.

Chapter 22's classic surprise: remove `memset` from `entity_create_player`,
run `make valgrind` (tests), see zero errors — then play the game under
memcheck and watch the bug appear. The lesson is excellent; deleting
production initialisation to learn it is not. This drill encodes
`test_suite_path` vs `covered_path` with an intentional uninitialised field.

## What this lesson asks of you

Read the coverage distinction. Reject gutting production `memset`. Then open
`TASK.md`, leave the intentional bug in `buggy_create` as designed, and make
sure `make check` still passes its harness while you can explain which path
tools can see.

## Foundations — quiet tools vs covered bugs

| Path | Calls `buggy_create`? | What tools can say |
| ---- | --------------------- | ------------------ |
| `test_suite_path` | No (uses a memset'd stand-in) | Quiet — like `run_tests` |
| `covered_path` | Yes, then reads `ready` | Can see the uninitialised use |

Production keeps `memset` (or equivalent full initialisation). The drill
keeps an intentional gap so you can *reason* about coverage without leaving
the game weaker.

## Worked example

**Step 1 —** `test_suite_path` passes and never touches the buggy creator.

**Step 2 —** `covered_path` constructs via `buggy_create` and reads `ready`
(analogous to party code clamping `hp`/`mp`).

**Step 3 — rejected wrong reading.** "Reintroduce the uninitialised read in
real `entity_create_player` and remove `memset` until I see valgrind scream."
That teaches coverage by vandalizing the lasting invariant the chapter just
added. Practice the coverage idea **here**; keep production initialised.

Optional (if `valgrind` is installed): run `./tool_coverage` under memcheck
after you understand which path executes — still never delete production
`memset` for practice.

## Check yourself

1. Why can a clean `make valgrind` on tests coexist with a dirty game path?
2. What does ASan often say about the same class of bug compared to memcheck
   with `--track-origins=yes`? (Chapter body contrasts them.)
3. Where should the lasting fix live — drill or `entity_create_player`?

## Key takeaways

- Tools report executed paths only.
- Clean tests ≠ clean program.
- Keep production `memset`; rehearse coverage in this folder.

## Lookup

- Chapter 22 hunt / Common errors
- `valgrind --track-origins=yes`; ASan limits on uninit

Now open `TASK.md`.
