# 02 — Loader invariants (validate at the boundary)

Known-spell menus treat the table as a **sorted prefix**: walk until level
exceeds the hero, then stop. That is O(prefix) and correct **only if** levels
are non-decreasing. An unknown effect name is a second boundary check — data
may select behaviour, not invent it.

Chapter 21's production loader keeps a sortedness check for that reason. An
old exercise idea was "remove the check and put a high-level spell first."
That is a lasting-path fork. This folder rehearses the diagnosis with
disposable fixtures.

## What this lesson asks of you

Read why prefix≠scan on unsorted data. Reject deleting production checks.
Then open `TASK.md` and complete the TODOs so `make check` passes against
`fixtures/`.

## Foundations — validate once, keep the hot path simple

| Check | Meaning |
| ----- | ------- |
| `is_sorted_by_level` | each level ≥ previous |
| `effect_known` | token ∈ {heal, damage, self_status, enemy_status, cure} |
| `validate_table` | reject unsorted or unknown; fill a reason string |
| `known_count_prefix` | stop at first too-high level (hot path) |
| `known_count_scan` | count all ≤ hero level (oracle for the lesson) |

On a sorted table, prefix and scan agree. On `fixtures/unsorted.txt`, they
disagree — the menu would show the wrong spell. That is the entire point of
keeping the production check.

## Worked example

**Step 1 — sorted fixture validates; prefix matches scan.**

**Step 2 — unsorted fixture fails `validate_table`; print the reason; note
prefix≠scan at hero level 2.**

**Step 3 — rejected wrong reading.** "Delete the sortedness check in
`magic.c` so I can see the bug in the real game." You will see it — and you
will leave the lasting loader weaker. Fixtures show the same disagreement
without gutting production. Ship content edits belong in `assets/`;
invariant experiments belong here.

## Check yourself

1. If levels are unsorted, what does a prefix `known_count` assume that is
   false?
2. Why snprintf a reason at the boundary instead of only returning 0?
3. Should production remove sortedness after you "understand" it?

## Key takeaways

- Validate-at-boundary; keep hot-path prefix counting simple.
- Unsorted data makes prefix≠scan — menus lie.
- Do not remove production sortedness to explore; use fixtures.

## Lookup

- Chapter 21 loader; `magic_known_count`
- Chapter 11: same fail-closed instinct for maps

Now open `TASK.md`.
