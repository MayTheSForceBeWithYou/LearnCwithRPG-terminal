# Appendix E: Stretch Goals

Four projects for after the course, chosen at Checkpoint F.

These are **sketches, not chapters.** Each gives you the design decisions,
the traps, and roughly where the code goes — then leaves you to write it.
That is the point: the course is over, and the next thing you build should
be built without someone reading over your shoulder.

They are ordered by effort. The first two are an evening each; the last is a
weekend and teaches the most.

| | Effort | Mostly teaches | Touches |
|---|---|---|---|
| [Bestiary](#1-bestiary) | small | data tables, save format growth | `battle`, `save`, `game` |
| [New Game+](#2-new-game) | small | serialisation design | `save`, `game`, `party` |
| [Boat](#3-a-boat) | medium | traversal rules, world gating | `map`, `entity`, `world` |
| [SDL2 port](#4-sdl2-port) | large | the payoff for `render.h` | one new file |

Before any of them: **branch.** `git switch -c bestiary`. The whole reason
this project has been committed chapter by chapter is so you can try
something, hate it, and get back.

---

## 1. Bestiary

A record of every enemy you have fought, viewable from the menu, filling in
as you play.

### What to build

One new bit of state: which enemy types have been seen. With four rosters of
four, plus three bosses, that is under 32 — so an `unsigned` used as a bit
set works, and slots into the save format as a single field.

```c
/* game.h */
unsigned seen_enemies;      /* one bit per enemy type */
```

Mark them where a battle starts, in `battle_begin` and `battle_begin_boss` —
not where one dies, or fleeing would leave gaps a player cannot explain.

Then a `MODE_BESTIARY` beside the other modes, listing each type with its
stats if seen and `???` if not.

### The interesting problem

Enemy types are currently anonymous: rosters are per-area arrays, and an
`EnemyType` has no id. A bit set needs a stable index for every enemy in the
game.

Two ways, and the choice is the exercise:

- **A flat table of all enemy types**, with the per-area rosters holding
  indices into it. One list, stable ids, and the bestiary is a loop over it.
  More rework.
- **Compute an id from `(area, index)`** — `area * 4 + index`. No rework,
  but the ids shift the moment a roster gains an entry, and every existing
  save is silently wrong.

The first is right. The second is how save formats rot.

### Traps

- **Bump `SAVE_VERSION`.** A new field means an old save must be rejected
  rather than misread — Chapter 20's rule, now applied to your own change.
- Validate the field on load like any other: a bit set with bits above your
  enemy count is a corrupt file, not a curiosity.
- Bosses should probably appear in a separate section, since three entries
  of `???` among sixteen is a spoiler in itself.

### Where to stop

Resist per-enemy kill counts, drop tables, and lore text. The version that
ships is the version that lists names and stats.

---

## 2. New Game+

Finish the game, then start again keeping your level, gear, and gold — with
the enemies scaled to match.

### What to build

The ending already runs through `MODE_ENDING`. Instead of setting
`running = 0` on the last page, offer the choice:

```
The Guild has another commission. Start again? (y/n)
```

Answering yes writes a save that keeps `level`, `xp`, `gold`, `equipped`
and the inventory, and resets `area`, `x`, `y`, and the fragments — then
restarts the run.

Add a `new_game_plus` count to the save so the game knows how many times
this has happened.

### The interesting problem

**Everything is trivial at level 15 in Grubbin Vale.** Slimes with 14 HP are
not a game.

Scale the enemies. The cleanest place is `battle_begin`, where the group is
built: multiply the type's HP, attack and XP by a factor derived from
`new_game_plus`. Copy the `EnemyType` into the `Enemy` rather than pointing
at the shared table — `Enemy.type` is a `const EnemyType *` today, so this
is a real design change and worth thinking about before typing.

Decide deliberately: does gear carry over? If it does, the Silver Rapier
trivialises Act I even scaled. Dragon Quest keeps equipment; many roguelikes
keep nothing but knowledge. Both are defensible; pick one and write down
why.

### Traps

- Reset the *fragments*, or the gates open immediately and the game is
  over before it starts. This is the bug you will actually hit.
- `SAVE_VERSION` again.
- Do not scale the config file's `enemy_damage` as well — the player set
  that, and silently multiplying it is taking their difficulty choice away.

---

## 3. A boat

A key item that makes water passable, opening terrain that was scenery until
you had it.

### What to build

A new tile type. `map.c` currently accepts `#`, `.` and `+`; add `~` for
water, walkable **only when the boat is carried**.

The check lives in `map_is_walkable`, which does not know about inventories
and should not learn:

```c
/* map.h */
int map_is_walkable(const Map *map, int x, int y);
int map_tile_needs(const Map *map, int x, int y);   /* required item, or -1 */
```

The caller — `explore_handle`, which already holds the whole `Game` —
combines the two. That keeps `map.c` free of game rules, which is the
property that has made it reusable for fourteen chapters.

### The interesting problem

**Map design becomes the hard part, not code.** A boat is only interesting
if the world was built with water in it, and this one was not. You will be
editing six `.map` files to add coastline that leads somewhere, which means
deciding what is out there.

That is the real work, and it is level design rather than programming. Do
not underestimate it, and do not let it turn into a new continent.

### Traps

- Test **every** existing map still loads. `map.c` rejects unknown tile
  characters, and the fuzzer will tell you immediately if you have broken
  the parser (`make fuzz`).
- A boat on land is the classic bug — you sail into a wall of grass.
  Decide whether the boat is a mode you are in or an item you hold; holding
  is far simpler, and Dragon Quest's shoreline rule (you may only *land*
  where the tile is walkable on foot) falls out of it naturally.
- Add the tile to the renderer, or your ocean is `?`.

---

## 4. SDL2 port

Replace the terminal with a window, tiles and sprites — without touching
game logic.

### Why this one is the finale

Chapter 8 built `render.h` for exactly this, and then never cashed it in.
Six functions:

```c
bool render_init(void);
void render_shutdown(void);
void render_clear(void);
void render_draw_tile(int x, int y, TileType t);
void render_draw_text(int x, int y, const char *s);
void render_present(void);
InputEvent input_poll(void);
```

Nothing above that line knows whether it is drawing to a terminal. If the
abstraction was honest, this port is one new file and a `Makefile` change,
and **`game.c` is not edited at all**.

If it is not honest, you will find out precisely where — and that discovery
is worth more than the window.

### What to build

`render_sdl.c` implementing the same seven functions, selected in the
`Makefile`:

```make
RENDERER ?= ncurses
ifeq ($(RENDERER),sdl)
    RENDER_SRC = render_sdl.c
    LDLIBS = -lSDL2 -lSDL2_ttf $(SAN)
else
    RENDER_SRC = render_ncurses.c
    LDLIBS = -lncursesw $(SAN)
endif
```

```bash
sudo pacman -S --needed sdl2-compat sdl2_ttf
make RENDERER=sdl
```

Note the name. Arch no longer ships an `sdl2` package: it was replaced by
**`sdl2-compat`**, an SDL2 API layer running on SDL3, which declares
`Provides: sdl2` so the headers and `-lSDL2` still work unchanged. Asking
for `sdl2` resolves through that provides entry rather than failing, but
naming the real package avoids the prompt.

This is Chapter 0's rule earning its keep: *verify package names against
the current repos rather than trusting a tutorial's memory* — including
this one's. It was written when `sdl2` was a real package.

Then, roughly:

- `render_init` — `SDL_Init(SDL_INIT_VIDEO)`, create a window and renderer,
  load a font with `TTF_OpenFont`.
- `render_draw_tile` — a coloured rectangle per `TileType` to start with.
  Sprites can come later; the boring version proves the abstraction first.
- `render_draw_text` — `TTF_RenderText_Blended` into a texture. Cache the
  textures or you will re-render every string every frame.
- `input_poll` — `SDL_PollEvent`, mapping `SDLK_w` and friends to the same
  `InputEvent` values.

### The interesting problem

**The game loop is currently blocking.** `getch()` waits for a key; the
program does nothing between keypresses, which is fine in a terminal and
wrong in a window — an SDL app that does not pump its event queue stops
redrawing and the compositor marks it unresponsive.

`input_poll` must return `INPUT_NONE` rather than waiting, and the loop has
to keep drawing. The existing loop already handles `INPUT_NONE` correctly,
so this may work unchanged — but it will spin at 100% CPU without a frame
cap. Add `SDL_Delay(16)`.

Coordinates are the other half. Chapter 9's camera talks in **character
cells** (Checkpoint A). Keep it that way and multiply by a tile size in the
renderer — do not let pixels leak upward into `camera.c`.

### Traps

- WSLg must be working. `echo $DISPLAY` should print something; if no
  window appears, `wsl --update` from PowerShell fixes most cases. See
  [Appendix A](a-wsl-troubleshooting.md).
- `render_draw_text` returning early on failure is how you get a blank
  window and no error. Check `TTF_OpenFont`.
- Run it under ASan the first time. New code doing new allocation is
  exactly where Chapter 22's habits pay.
- Keep both renderers building. A port that breaks the terminal version has
  not demonstrated anything about the abstraction.

---

## And then

None of these is the real stretch goal. The real one is the thing you
thought of around Chapter 16 and wrote down somewhere.

You now know enough C to build it, and — more usefully — enough to find out
why it does not work. That was the whole point.
