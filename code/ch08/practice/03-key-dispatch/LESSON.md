# 03 — Key dispatch table

**Switchboard vs hard-wired lamps.** Mapping `KEY_UP` and `'w'` onto the same
`INPUT_UP` action is one switchboard row. Wiring each lamp directly to
movement code duplicates the path and makes a second input source (gamepad,
demo replay) painful.

Chapter 8's lasting UX work puts arrow keys into the real `input_poll`. This
drill owns the table-driven idea in isolation: raw key codes in, `InputEvent`
out, no ncurses required. By the end you should map WASD and fake arrow codes
to the same actions, leave unknowns as `INPUT_NONE`, and reject the wrong
reading that hard-wires movement inside every key branch.

## What this lesson asks of you

Read the switchboard design. Reject scattering movement logic through key
cases. Then open `TASK.md` and complete `input_from_key` in `main.c` so
`make check` asserts WASD, arrows, quit, and unknown keys.

## Foundations — raw key in, action out

Game update code should ask "what did the player *mean*?" — not "which
physical key did ncurses report?" Two shapes for that translation:

```c
/* Switch — fine for a small keymap */
switch (key) {
    case 'w': case 'W': case KEY_UP: return INPUT_UP;
    /* ... */
    default: return INPUT_NONE;
}
```

```c
/* Explicit table of {key, event} pairs — same single-place rule */
```

Both keep the decision in **one place**. Callers of `input_from_key` (or the
chapter's `input_poll`) receive `INPUT_UP`, not a soup of `'w'` and
`KEY_UP` checks sprinkled through movement code.

Fake arrow codes in this drill (`1000`…`1003`) stand in for ncurses
`KEY_UP` and friends so you need no `-lncursesw` here. The lasting chapter
exercise wires the real macros into the game's input module.

## Worked example

**The situation.** Players expect both WASD and arrows to move. Quit is `'q'`.
Unknown keys must not invent motion.

**Step 1 — one function owns the map.**

```c
InputEvent e = input_from_key(key);
/* update uses e, never re-checks 'w' vs KEY_UP */
```

**Step 2 — many keys, one action.**

`'w'`, `'W'`, and `KEY_UP` all return `INPUT_UP`. Adding a gamepad "D-pad up"
later is another case (or table row) in this function — not a second copy of
the move-north logic.

**Step 3 — rejected wrong reading.** "Just put `player.y--` inside the
`case KEY_UP:` and another `player.y--` inside `case 'w':`." That hard-wires
lamps to the movement code. A demo replay that synthesizes `INPUT_UP` events
cannot reuse the path; a second device duplicates every branch. The
switchboard maps keys to actions; update consumes actions.

## Distinctions worth keeping straight

- **Key vs action** — `KEY_UP` / `'w'` are devices; `INPUT_UP` is meaning.
- **Drill vs lasting path** — practice the table here; wiring real `KEY_*`
  into `input_poll` is the chapter's durable UX step, not a throwaway.
- **Unknown keys** — return `INPUT_NONE`; do not treat every leftover as quit
  or as motion.
- **Case folding** — `'W'` and `'w'` are both switchboard rows (or fall
  through the same case), not separate movement implementations.

## Check yourself

1. Player presses arrow-up. Should `update` see `KEY_UP` or `INPUT_UP`?
2. You add a second binding for up (say a gamepad button). Which file/function
   should grow — the keymap, or every place that moves the player?
3. Why is "paste `player.y--` into both `'w'` and `KEY_UP` cases" the wrong
   lasting habit even if the avatar moves correctly today?
4. After this drill, where do real ncurses arrow macros belong — this
   practice folder forever, or the game's `input_poll`?

## Key takeaways

- Raw keys map to `InputEvent` in one place; update logic consumes actions.
- Multiple physical keys may share one action (WASD + arrows).
- Hard-wiring movement inside each key case duplicates paths and blocks
  alternate input sources.
- Practice the table here; ship arrow support on the lasting game path.

## Lookup (not the lesson)

- Chapter 8 exercise 2: arrows in live `input_poll`
- `ANALOGIES_FOR_CHAPTER.md` — switchboard vs hard-wired lamps
- ncurses `KEY_UP` / `KEY_DOWN` / `KEY_LEFT` / `KEY_RIGHT` (game path only)

Now open `TASK.md` and do the practice.
