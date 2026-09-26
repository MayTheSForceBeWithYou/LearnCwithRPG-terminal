# 02 — Present at the boundary (not raw refresh)

**Kitchen ticket window.** Game code slips a finished frame through the
window (`render_present`). The cook behind the window (`refresh` / backend
flush) is not someone the dining room should know by name. If waiters walk
into the kitchen and flip the grill themselves, a new kitchen (SDL backend)
means retraining every waiter. One-line wrappers exist so the *call sites*
stay stable when the implementation grows.

Chapter 8 wraps ncurses `refresh()` behind `render_present()` so game logic
never names the backend. The old exercise temptation — delete the wrapper and
call `refresh()` from `draw_world` — demonstrates boundary loss by vandalizing
the lasting game. This drill keeps the lesson and moves the damage into a
tiny stub you can build two ways: correct (`make`) and leaky (`make leak`,
for reading only).

## What this lesson asks of you

Trace which translation unit may know about a backend flush. Complete
`draw_world` so it calls `render_present`. Build the correct program. Inspect
the leak target and answer the reading questions — without deleting
`render_present` from the chapter game.

## Foundations — who is allowed to know

Three files, two roles:

| File | Role | May call backend flush? |
| ---- | ---- | ----------------------- |
| `render_stub.h` | Opaque contract: `render_draw`, `render_present` | Declares present; never declares `fake_refresh` / `refresh` |
| `render_stub.c` | Implementation (stand-in for `render_ncurses.c`) | Yes — that is its job |
| `game_stub.c` | Game / `draw_world` stand-in | No — only `render_*` |

```
game_stub.c  --calls-->  render_present()  --inside stub-->  backend flush
                            ^
                            |  render_stub.h is the only #include game needs for drawing
```

If `game_stub.c` calls `fake_refresh()` (or, in the real chapter, ncurses
`refresh()`), it must either declare that symbol itself or `#include` a
header that exposes it. Either way the game translation unit now **depends
on the backend**. Swapping `render_stub.c` for a future `render_sdl.c` stops
being "change one Makefile line": you must also rewrite every leaked call in
game logic.

`render_present` exists so that dependency arrow never grows that direction.

## Worked example

**The situation.** A one-glyph "world." Correct `draw_world` draws, then
presents. The stub records present calls with a counter (no ncurses required
for the drill).

**Step 1 — correct wiring (`make`).**

`game_stub.c` includes only `render_stub.h` and ends `draw_world` with
`render_present()`. `render_stub.c` implements `render_present` by flushing
internally. Link them. Run. Harness expects two present calls. Reading the
sources: `game_stub.o` references `render_present`, not `fake_refresh`.

**Step 2 — leak target (`make leak`) — read, do not copy into the game.**

`game_leak.c` calls `fake_refresh()` directly. The Makefile builds that
variant with `-DEXPOSE_FAKE_REFRESH` so `render_stub.c` exports the symbol
for the demo. The program may still "work" today — same flush count, same
behavior. What you lost is the architecture: `game_leak.c` now hard-wires the
stub's private name. In the chapter game the analogous leak is
`#include <ncurses.h>` plus `refresh()` inside `draw_world` after deleting
`render_present`.

**Step 3 — rejected wrong reading.** "It's a one-line wrapper, so deleting it
simplifies the code; we can always add it back." Wrappers that preserve an
opaque boundary are not noise. The cost shows up later, when a second backend
appears or when a third file also starts calling `refresh()` "just this
once." The chapter's lasting path keeps `render_present`. This folder's
`make leak` is a museum exhibit, not a refactor script. Do **not** delete
`render_present` and call `refresh` from `draw_world` in the playable game.

## Distinctions worth keeping straight

- **Behavioral sameness vs dependency** — calling `refresh()` can look
  identical on screen while permanently coupling game code to ncurses.
- **Header surface** — if `render_stub.h` never mentions `fake_refresh`,
  correct game code cannot call it without reaching past the contract.
- **Stub vs ncurses** — this drill fakes the backend with a counter so you
  need no `-lncursesw`. The dependency lesson is the same shape.
- **Exercise hygiene** — learning "what you lose" does not require deleting
  production symbols. Side targets exist for that.

## Check yourself

1. Which file in the correct build is allowed to call the backend flush /
   `fake_refresh`?
2. If `game_stub.c` called ncurses `refresh()` in the real project, which
   `#include` would suddenly appear in game logic, and why does that block a
   clean `render_sdl.c` swap?
3. Does "the leak binary still increments the present counter" mean the
   abstraction was unnecessary? Why or why not?
4. Should you delete `render_present` from the chapter game after this drill?

## Key takeaways

- `render_present` is the game-facing name for "flush the backend now."
- Game translation units call `render_*` only; backend names stay in the
  render `.c` file.
- A leak can run fine today and still be the wrong lasting dependency.
- Inspect `make leak` here; leave the playable game's wrapper in place.

## Lookup (not the lesson)

- Chapter 8: `render.h` / `render_present` / `refresh()` pairing
- Chapter 4: `.h` / `.c` split and what `#include` means for dependencies
- `make leak` recipe in this folder's `Makefile`

Now open `TASK.md` and do the practice.
