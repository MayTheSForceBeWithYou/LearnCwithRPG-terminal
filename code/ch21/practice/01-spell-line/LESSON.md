# 01 — Parse one spell line

A spell row is a **ticket**: level, MP cost, magnitude, effect name, then a
display name that may contain spaces. The loader's first job is to parse one
line fail-closed before it worries about whole-table invariants (those are
`02-loader-invariants`).

Chapter 21 ships spells as data. By the end of this drill you should parse a
whitespace-separated row into fields, keep multi-word names intact, reject
garbage, and reject the wrong reading that says "just `strtok` forever and
hope the name has no spaces."

## What this lesson asks of you

Read the field layout and the worked example. Reject silent partial parses.
Then open `TASK.md` and complete `parse_spell_line` so `make check` passes.

## Foundations — fields, then the name tail

Canonical shape (simplified vs full chapter columns — this drill's harness):

```text
4 4 45 heal Second Wind
│ │ │  │    └─ name (may contain spaces)
│ │ │  └─ effect token
│ │ └─ magnitude
│ └─ mp
└─ level
```

A practical `sscanf` pattern uses a scanset for the name tail after the
effect token (`%31[^\n]`), not "five `%s` tokens."

Malformed lines fail closed: return 0; do not leave half-filled structs for
callers to treat as real spells.

## Worked example

**Step 1 — happy path.**

```c
Spell s;
parse_spell_line("4 4 45 heal Second Wind\n", &s);
/* level 4, mp 4, mag 45, effect "heal", name "Second Wind" */
```

**Step 2 — reject garbage.**

```c
parse_spell_line("nope\n", &s);  /* → 0 */
```

**Step 3 — rejected wrong reading.** "I'll split on spaces and take the
last tokens as the name by joining leftovers in a loop I invent later."
That works until it does not — and it duplicates policy the scanset already
expresses. Prefer one parse contract, fail closed, leave table-wide
sortedness/effects to drill 02.

## Check yourself

1. Why is `%s` for the name field insufficient when names contain spaces?
2. What should happen to `out` fields if the line is malformed — partial
   write OK, or fail closed before callers trust it?
3. How does this drill differ from `02-loader-invariants`?

## Key takeaways

- One-line parse is its own contract; fail closed.
- Name tails need a scanset (or equivalent), not blind `%s`.
- Table invariants (sorted levels, known effects) are the next drill — do
  not remove production checks to explore them.

## Lookup

- Chapter 21 spell file format; `magic_load_spells`
- `sscanf` scansets

Now open `TASK.md`.
