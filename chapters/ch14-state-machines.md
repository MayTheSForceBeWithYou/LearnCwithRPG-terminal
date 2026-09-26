# Chapter 14: State Machines

## Where we are

Chapter 13 gave you three areas, a cast, and doors between them — and left
`main.c` with a `switch` inside a `switch` inside the game loop, with
movement logic buried three levels deep. It works. It was already unpleasant
to read when there were only two modes. This chapter is the payoff: function
pointers, a dispatch table, and a `MODE_MENU` that gets added *without*
touching a single line of the modes that already existed.

## The problem

Look honestly at what Chapter 13 left behind:

```c
switch (mode) {
    case MODE_EXPLORE: {
        int dx = 0, dy = 0;
        switch (event) { /* ... */ }
        if (dx != 0 || dy != 0) {
            /* NPC check, walkability check, warp check, all nested here */
        }
        break;
    }
    case MODE_DIALOG:
        if (event != INPUT_NONE) { mode = MODE_EXPLORE; }
        break;
}
```

Now imagine adding a menu, then a shop, then a battle. Every one is another
`case` block in the same function, in the same file, sharing the same local
variables. The function grows past any reasonable length (Chapter 4's ~50
line guideline is long gone), and — the real cost — **every mode can see and
touch every other mode's state**, because they're all in one scope. That's
not a style complaint; it's how a bug in the menu code ends up corrupting
movement.

What you want is for each mode to be a self-contained pair of functions, and
for the game loop to know nothing about which mode is active.

## C concept spotlight

### A pointer to a function

Functions live in memory, at addresses, just like variables — and a pointer
can hold one:

```c
static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }

int (*op)(int, int) = add;
printf("op = add: %d\n", op(3, 4));

op = sub;
printf("op = sub: %d\n", op(3, 4));
```

```
op = add: 7
op = sub: -1
```

`int (*op)(int, int)` declares `op` as *a pointer to a function taking two
`int`s and returning `int`*. Read it from the inside out: `op` is a pointer
(`*op`), to a function (`(...)`), taking `(int, int)`, returning `int`. The
parentheses around `*op` are load-bearing — without them, `int *op(int, int)`
declares a *function returning `int *`*, which is a completely different
thing. This is C's most notorious declaration syntax, and reading it
inside-out is the trick that makes it manageable.

Calling through the pointer is just `op(3, 4)`. You may also see `(*op)(3, 4)`
in older code; both are legal and identical.

### `typedef` makes it readable

Nobody wants `int (*)(int, int)` scattered through their code:

```c
typedef int (*BinaryOp)(int, int);

BinaryOp ops[] = { add, sub, mul };
const char *names[] = { "add", "sub", "mul" };

for (int i = 0; i < 3; i++) {
    printf("%s(6, 7) = %d\n", names[i], ops[i](6, 7));
}
```

```
add(6, 7) = 13
sub(6, 7) = -1
mul(6, 7) = 42
```

Note the odd shape of a function-pointer `typedef`: the name being defined
(`BinaryOp`) sits *in the middle* of the declaration, where the variable name
would go — not at the end like `typedef struct { ... } Player;`. That's
consistent with how C declarations work (the typedef name occupies the
position of the thing being named), but it looks wrong the first several
times you see it.

An **array of function pointers** is the whole idea of this chapter in one
line. `ops[i](6, 7)` selects a function by index and calls it — a `switch`
with three cases, replaced by one array access.

### Dispatch tables

Combine that with Chapter 13's enum-as-index and you get a **dispatch
table**: an array indexed by state, each entry holding the functions for
that state.

```c
typedef void (*DrawFn)(const Game *game);
typedef void (*HandleFn)(Game *game, InputEvent event);

typedef struct {
    const char *name;
    DrawFn draw;
    HandleFn handle;
} ModeHandler;

static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  },
    [MODE_MENU]    = { "menu",    menu_draw,    menu_handle    }
};
```

And the entire dispatch becomes two lines with no branching at all:

```c
void game_draw(const Game *game)
{
    render_clear();
    mode_table[game->mode].draw(game);
    render_present();
}

void game_handle(Game *game, InputEvent event)
{
    mode_table[game->mode].handle(game, event);
}
```

That's the refactor. Adding a mode is adding an enum value and a table row —
exactly the same shape as adding an area in Chapter 13, applied to behaviour
instead of data.

### What you gain, concretely

- **Each mode is isolated.** `menu_handle` cannot accidentally modify a local
  variable belonging to explore mode, because there are no shared locals any
  more — only the explicit `Game *`.
- **Adding a mode touches nothing existing.** No `switch` to extend, no
  function to grow.
- **The functions can be `static`.** Every handler in `game.c` is private to
  that file (Chapter 4), so the rest of the program can't call
  `menu_handle` directly even by accident. Only the table can.

### The cost, honestly

Function pointers are not free of downsides, and it's worth being clear about
them before you fall in love with the technique:

- **The compiler can't see where control goes.** With a `switch`, GCC knows
  every possible destination and can warn about missing cases; with a table,
  it can't. If you add `MODE_SHOP` to the enum and forget the table row, the
  compiler will not tell you. (Exercise 2 addresses this.)
- **A wrong or missing entry crashes at runtime**, not compile time. A table
  entry left as `NULL` is a null function pointer, and calling it is a
  segfault:
  ```
  $ ./nullfp
  $ echo $?
  139
  ```
- **Indirection costs a little performance** — the CPU can't predict the
  target as easily as a direct call. For a game loop running at human speed,
  this is irrelevant; it's worth knowing it exists, not worth worrying about.

Use dispatch tables when the set of states is open-ended and each one is
substantial. Use a `switch` when there are three tiny cases that will never
grow. Chapter 13's mode handling was the former pretending to be the latter.


### Fail closed on the dispatch table

A `NULL` `draw` or `handle` is a landmine that detonates only when a player
reaches that mode. `game_validate_modes` at startup (practice
`03_validate_handlers`, then the real function) fails the run before any
input. Same spirit as Chapter 13's world validate: table-driven systems
need table-shaped checks.


## Apply it

### The `Game` struct

All the loose variables from `main` move into one place, `game.h`:

```c
typedef enum {
    MODE_EXPLORE,
    MODE_DIALOG,
    MODE_MENU,
    MODE_COUNT
} GameMode;

typedef struct {
    World *world;
    Player player;
    Camera camera;
    Dialog dialog;
    GameMode mode;
    int menu_index;
    int running;
} Game;

Game *game_create(void);
void game_destroy(Game *game);

void game_draw(const Game *game);
void game_handle(Game *game, InputEvent event);
```

This is why the handlers can be isolated: everything they might need is
reachable through the single `Game *` they're handed. `running` replaces the
old `while (event != INPUT_QUIT)` condition, so any mode can end the game by
setting a flag rather than by returning a magic value up through the loop.

### Two new input events

The menu needs a way to open and a way to confirm. In `input.h`:

```c
typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_MENU,
    INPUT_CONFIRM,
    INPUT_QUIT
} InputEvent;
```

And in `input.c`:

```c
case 'm': case 'M': return INPUT_MENU;
case '\n': case '\r': case ' ': return INPUT_CONFIRM;
```

Note this changes nothing about how input *works* — `input_poll` still hides
ncurses behind the abstraction from Chapter 8. Adding a key is a one-line
change in exactly one file, which is that chapter's payoff arriving on
schedule.

### Three modes, three pairs of functions

Explore mode, in `game.c` — the same logic as Chapter 13, but now in its own
function with a name:

```c
static void explore_draw(const Game *game)
{
    draw_area_tiles(game);
    render_draw_text(0, VIEW_HEIGHT + 1, "w/a/s/d move, m menu, q quit");
}

static void explore_handle(Game *game, InputEvent event)
{
    int dx = 0;
    int dy = 0;

    switch (event) {
        case INPUT_UP:      dy = -1; break;
        case INPUT_DOWN:    dy = 1;  break;
        case INPUT_LEFT:    dx = -1; break;
        case INPUT_RIGHT:   dx = 1;  break;
        case INPUT_MENU:    game->mode = MODE_MENU; return;
        case INPUT_CONFIRM: return;
        case INPUT_QUIT:    game->running = 0; return;
        case INPUT_NONE:    return;
    }

    int target_x = game->player.x + dx;
    int target_y = game->player.y + dy;

    const Npc *npc = world_npc_at(game->world, target_x, target_y);
    if (npc != NULL) {
        dialog_wrap(&game->dialog, npc->speech, TEXTBOX_WIDTH);
        game->mode = MODE_DIALOG;
        return;
    }

    if (map_is_walkable(world_current_map(game->world), target_x, target_y)) {
        entity_move(&game->player, dx, dy);

        const Warp *warp = world_warp_at(game->world,
                                         game->player.x, game->player.y);
        if (warp != NULL) {
            game->world->current = warp->destination;
            game->player.x = warp->destination_x;
            game->player.y = warp->destination_y;
        }
    }
}
```

There's still a `switch` here — on the *input event*, which is a genuinely
fixed, small set. That's the right tool for that job. The `switch` this
chapter eliminated was the one on *mode*, which is the open-ended one.
Knowing which is which is the actual skill.

Dialog mode, which is now trivially small:

```c
static void dialog_handle(Game *game, InputEvent event)
{
    if (event == INPUT_QUIT) {
        game->running = 0;
        return;
    }

    if (event != INPUT_NONE) {
        game->mode = MODE_EXPLORE;
    }
}
```

And menu mode — **entirely new code that required changing nothing else**:

```c
static const char *const menu_items[] = {
    "Status",
    "Commission",
    "Close menu"
};

#define MENU_ITEM_COUNT ((int)(sizeof menu_items / sizeof menu_items[0]))

static void menu_draw(const Game *game)
{
    draw_area_tiles(game);

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        char line[32];
        snprintf(line, sizeof line, "%c %s",
                 i == game->menu_index ? '>' : ' ', menu_items[i]);
        render_draw_text(1, VIEW_HEIGHT + 1 + i, line);
    }
}

static void menu_handle(Game *game, InputEvent event)
{
    switch (event) {
        case INPUT_UP:
            game->menu_index--;
            if (game->menu_index < 0) {
                game->menu_index = MENU_ITEM_COUNT - 1;
            }
            break;

        case INPUT_DOWN:
            game->menu_index++;
            if (game->menu_index >= MENU_ITEM_COUNT) {
                game->menu_index = 0;
            }
            break;

        case INPUT_CONFIRM:
            /* ... show status, show commission, or close ... */
            break;

        /* ... */
    }
}
```

`sizeof menu_items / sizeof menu_items[0]` is the standard C idiom for "how
many elements are in this array" — total bytes divided by bytes per element.
It only works on a real array, not a pointer (where `sizeof` would give the
pointer's size), so it's safe here but would silently break if `menu_items`
were passed into a function first. The wrap-around arithmetic means pressing
up on the first item lands on the last, which is what every RPG menu does.

### The game loop, finally

`main.c` shrinks to almost nothing:

```c
while (game->running) {
    const Map *map = world_current_map(game->world);

    camera_center_on(&game->camera, game->player.x, game->player.y,
                     map->width, map->height);

    game_draw(game);
    game_handle(game, input_poll());
}
```

Compare that to Chapter 13's nested switches. `main` no longer knows that
modes exist. It draws, it polls, it hands the event off — and everything
about *what that means* lives in `game.c` behind the table.

Add `game.c` to `SOURCES`.

## Compile and run

```bash
make clean
make
./game
```

Press `m` to open the menu, `w`/`s` to move the cursor, Enter or space to
confirm:

```
####################
#...................
#.@####...####...###
#..####...####...###
#..####...####...###
#...................
#.......o...........
#..####...####......
#..####...####......
#..####...####......
Grubbin Vale
   Status
 > Commission
   Close menu
```

Confirm "Commission" and the dispatch table routes you into dialog mode,
reusing the exact same text box the NPCs use:

```
Grubbin Vale
 Recover four
 fragments of the
 Crown of Hollis.
 Payment on
```

"Status" builds its line with `snprintf` from live player data:

```
Grubbin Vale
 HP 20  MP 8  ATK 6
 DEF 4
```

Any key returns to explore mode; `q` quits from any mode. And the sanitizer:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

Silence.

## What just happened

`game->mode` is now an index, and the table turns that index into behaviour:

```
  game->mode = MODE_MENU  (2)
        |
        v
  mode_table[2]
  +-----------------------------------------+
  | name   = "menu"                          |
  | draw   ---------> menu_draw()   (code)   |
  | handle ---------> menu_handle() (code)   |
  +-----------------------------------------+
             |                 |
   game_draw() calls it   game_handle() calls it

  mode_table[0] -> explore_draw / explore_handle
  mode_table[1] -> dialog_draw  / dialog_handle
  mode_table[2] -> menu_draw    / menu_handle
```

The stored value is the *address of the function's machine code* — the same
code the linker placed in your executable back in Chapter 1. A function
pointer is a pointer to instructions rather than data, which is the only
genuinely new idea here; everything else is Chapter 7's pointers and Chapter
13's tables, combined.

## Common errors

**Missing parentheses in the declaration:**

```c
int *op(int, int);      /* a FUNCTION returning int*, not a pointer */
int (*op)(int, int);    /* a POINTER to a function returning int */
```

The first compiles happily as a function *declaration* and then fails
confusingly when you try to assign to it. When a function-pointer
declaration doesn't behave, check the parentheses before anything else.

**Assigning a function with the wrong signature:**

```
wrongsig.c: In function ‘main’:
wrongsig.c:9:17: error: initialization of ‘Handler’ {aka ‘void (*)(int)’} from incompatible pointer type ‘void (*)(void)’ [-Wincompatible-pointer-types]
    9 |     Handler h = takes_nothing;
      |                 ^~~~~~~~~~~~~
wrongsig.c:5:13: note: ‘takes_nothing’ declared here
```

A hard error, and a genuinely useful one — this is the compiler enforcing
that every entry in your dispatch table has the shape the table promises.

**A `NULL` entry in the table:**

```c
static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  }
    /* forgot MODE_MENU */
};
```

Any entry you don't initialize is zero-filled, which for a function pointer
means `NULL`. Pressing `m` to enter that mode calls it, and the process dies
on signal 11 — a segmentation fault:

```
$ ./game
[the game starts normally, then dies the moment you press m]
$ echo $?
139
```

**Nothing warns you about the missing entry.** In the real experiment behind
this section, GCC emitted only this:

```
game.c:155:13: warning: ‘menu_handle’ defined but not used [-Wunused-function]
game.c:143:13: warning: ‘menu_draw’ defined but not used [-Wunused-function]
```

— which is a hint, but an unreliable one: it appears only because those
handlers became unreferenced. Had they been mentioned anywhere else, you'd
get a completely silent build and a crash at runtime. This is the price of
trading a `switch` (which `-Wswitch` checks, as you saw in Chapter 6) for a
table (which nothing checks). Exercise 2 is about buying that safety back.

## Exercises

> **Practice drills:** `code/ch14/practice/` (`01_fnptr_basics`–
> `04_tiny_fsm`) before exercise 2. Learn the FSM shape in the tiny harness
> — do not rip up `game.c`'s table as an experiment.

1. *Practice.* Finish all four drills, especially `04_tiny_fsm`.
2. *Durable — validate modes.* Implement `game_validate_modes` (non-NULL
   `draw`/`handle` for every `MODE_*`). Call it from startup; refuse to
   run on failure.
3. *Durable — new mode.* Add `MODE_GAME_OVER` (message, only `q`). Count
   how many *existing* functions you modified vs Chapter 13's nested
   `switch`. (Menu item "Party" is optional content — fine as data in
   `menu_items`, not a substitute for the mode validate.)
4. *Open-ended:* Sketch `on_enter` / `on_exit` on `ModeHandler` and a
   `game_set_mode` that runs them. Resetting `menu_index` on menu open is
   the motivating example.

<details>
<summary>Solutions</summary>

1. Practice solutions under `solutions/`.
2. Loop `0 .. MODE_COUNT-1`; stderr on NULL; return false → main exits.
3. Enum entry + one table row + two functions — not a hunt through nested
   switches. Party item: usually only `game.c` menu arrays/switch.
4. `ModeHandler` gains `on_enter`/`on_exit`; `game_set_mode` calls exit on
   old, assign, enter on new (NULL-safe).

</details>

## Next up

Your game has a world, a cast, and clean modes to move between — everything
except a reason to be afraid of the roads. **Checkpoint C comes next**:
before Chapter 15, you'll decide how combat actually works — turn order,
the damage formula, how encounters start, and how forgiving the whole thing
should be.
