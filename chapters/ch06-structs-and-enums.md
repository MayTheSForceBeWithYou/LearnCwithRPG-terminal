# Chapter 6: Structs and Enums

## Where we are

The map draws, but nobody's standing on it. Your hero's stats are still
five separate variables — `hp`, `mp`, `attack`, `defense`, `rank` — that
happen to travel together through `character_intro` but aren't actually
*one thing* as far as C is concerned. This chapter fixes both problems:
`struct` bundles related data into a single named type, `enum` gives a
small fixed set of choices (like a rank) real names instead of magic
characters, and by the end your hero appears as an `@` standing on the map.

## The problem

"Five variables that happen to be related" is fragile — nothing stops you
from updating `hp` in one place and forgetting `mp` exists, or passing the
wrong variable to the wrong function since they're just individually-typed
`int`s with no connection between them. You need one type that *is* a
player: name, position, and stats, all together, passed around as a single
value.

## C concept spotlight

### `struct`: bundling related data

```c
typedef struct {
    int x;
    int y;
} Point;

int main(void)
{
    Point start = {3, 4};
    printf("start.x = %d, start.y = %d\n", start.x, start.y);

    Point moved = start;
    moved.x += 10;
    printf("start.x = %d (unchanged), moved.x = %d\n", start.x, moved.x);

    return 0;
}
```

```
start.x = 3, start.y = 4
start.x = 3 (unchanged), moved.x = 13
```

`struct { int x; int y; }` defines a new type with two named **members**,
accessed with `.`. `Point moved = start;` copies *both* members at once —
this is worth staring at, because it's the single most important thing to
understand about structs in C: assigning one struct to another copies
every byte, not a reference or a shared connection. Changing `moved`
afterward has no effect on `start` whatsoever, exactly like assigning
`int b = a; b = 99;` never changes `a`. Keep this firmly in mind; it's
about to explain a bug you're going to write on purpose in a few minutes.

### `typedef`: naming the type once

Without `typedef`, C requires writing `struct Point` every time you want
the type, not just `Point`:

```c
struct Point {
    int x;
    int y;
};

struct Point start = {3, 4};   /* the "struct " part is mandatory here */
```

`typedef struct { ... } Point;` (note: no name between `struct` and `{`,
just the `typedef`ed name at the end) collapses that into one clean
identifier, `Point`, used everywhere else. This course always uses the
`typedef` form; you'll still recognize the longer form when you see it in
other people's code.

### `enum`: naming a fixed set of choices

Chapter 2 stored your hero's rank as `char rank = 'E';` — a single
character standing in for a concept ("this is the worst rank") with
nothing enforcing that only sensible letters get used. An **enum** gives a
small, fixed set of named integer constants:

```c
typedef enum {
    RANK_E,
    RANK_D,
    RANK_C,
    RANK_B,
    RANK_A,
    RANK_S
} Rank;
```

```c
Rank r = RANK_C;
printf("RANK_E = %d\n", RANK_E);   /* 0 */
printf("RANK_C = %d\n", RANK_C);   /* 2 */
printf("r = %d\n", r);             /* 2 */
```

Under the hood, an enum really is just `int` — `RANK_E` is `0`, `RANK_D`
is `1`, and so on, counting up automatically unless you assign specific
values yourself (`RANK_C = 10,` would make everything after it count up
from 10 instead of 2). The value here isn't the point; the *name* is. Once
you have `RANK_C` as a name, `if (p.rank == RANK_C)` reads as English,
where `if (p.rank == 2)` would leave you re-deriving "wait, what was rank 2
again?" every time you read it. Note also that C has no built-in way to
turn an enum value back into a readable string automatically — `printf`
with `%d` shows the raw number. Converting `Rank` into a printable letter
takes a small function of your own, using exactly the `switch` statement
Chapter 3 taught.

### Composition

Structs can contain arrays, other structs, and enums as members — this is
**composition**, and it's how you build up a "hero" from smaller pieces
you already understand:

```c
typedef struct {
    char name[64];
    int x;
    int y;
    int hp;
    int mp;
    int attack;
    int defense;
    Rank rank;
} Player;
```

`Player` is a `char` array, five `int`s, and a `Rank` (itself really an
`int`), all traveling together as one value. Nothing here is a new
mechanism — it's every type you already know, composed into a shape that
matches what a "player" actually is.

## Apply it

Retire `character.h`/`character.c` in favor of a new module, `entity.h` /
`entity.c` — matching this course's target module layout, since player
representation is `entity`'s job from here forward.

`entity.h`:

```c
#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    RANK_E,
    RANK_D,
    RANK_C,
    RANK_B,
    RANK_A,
    RANK_S
} Rank;

typedef struct {
    char name[64];
    int x;
    int y;
    int hp;
    int mp;
    int attack;
    int defense;
    Rank rank;
} Player;

Player entity_create_player(void);
void entity_print_sheet(Player p);
void entity_move(Player p, int dx, int dy);

#endif
```

`entity.c` — `entity_create_player` replaces `character_intro`'s job,
now returning a fully-populated `Player` instead of just printing:

```c
#include <stdio.h>
#include "entity.h"

static char rank_to_char(Rank rank)
{
    switch (rank) {
        case RANK_E: return 'E';
        case RANK_D: return 'D';
        case RANK_C: return 'C';
        case RANK_B: return 'B';
        case RANK_A: return 'A';
        case RANK_S: return 'S';
    }
    return '?';
}

Player entity_create_player(void)
{
    Player p;

    printf("What is your name, hero? ");
    fgets(p.name, sizeof p.name, stdin);

    p.x = 1;
    p.y = 1;
    p.hp = 20;
    p.mp = 8;
    p.attack = 6;
    p.defense = 4;
    p.rank = RANK_E;

    return p;
}

void entity_print_sheet(Player p)
{
    float power_rating = (float)(p.attack + p.defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", p.name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", p.hp);
    printf("MP:     %d\n", p.mp);
    printf("ATK:    %d\n", p.attack);
    printf("DEF:    %d\n", p.defense);
    printf("Rank:   %c\n", rank_to_char(p.rank));
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}
```

`entity_print_sheet` takes `Player p` — a full copy of the struct — which
is harmless here because it only *reads* `p`. Now add one more function,
written to look like an obviously correct way to move the player:

```c
void entity_move(Player p, int dx, int dy)
{
    /* This looks right. It compiles clean. It does not work -- p is a
       copy, so changing it here changes nothing the caller can see.
       Chapter 7 explains exactly why, and fixes it for good. */
    p.x += dx;
    p.y += dy;
}
```

This is deliberately, honestly broken — hold that thought for "Compile and
run" below.

Update `map.h` and `map.c` so the map can show where the player is
standing:

```c
/* map.h -- signature change */
void map_print(int player_x, int player_y);
```

```c
/* map.c -- map_print's new body */
void map_print(int player_x, int player_y)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            if (row == player_y && col == player_x) {
                printf("@");
            } else {
                printf("%c", world[row][col]);
            }
        }
        printf("\n");
    }
}
```

And `main.c`, tying it together, including the deliberately-broken move
attempt:

```c
#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Player player = entity_create_player();
    entity_print_sheet(player);

    printf("\n");
    printf("The village gate creaks open.\n");
    printf("\n");
    map_print(player.x, player.y);

    printf("\n");
    printf("You try to step east...\n");
    entity_move(player, 1, 0);
    printf("Player position: (%d, %d)\n", player.x, player.y);
    printf("\n");
    map_print(player.x, player.y);

    duel_run();

    return 0;
}
```

Finally, update the `Makefile`'s `SOURCES` to drop `character.c` and add
`entity.c`:

```make
SOURCES = main.c entity.c map.c duel.c
```

(Delete `character.h` and `character.c` from your project — `entity`
fully replaces them.)

## Compile and run

```bash
make clean
make
./game
```

Zero warnings. Expected output (name `Elowen`):

```
****************************************
*                                      *
*           UNTITLED RPG               *
*                                      *
****************************************
What is your name, hero? Elowen

Welcome, Elowen
----------------------------------------
HP:     20
MP:     8
ATK:    6
DEF:    4
Rank:   E
Power:  5.0
----------------------------------------

The village gate creaks open.

##########
#@.......#
#..####..#
#........#
##########

You try to step east...
Player position: (1, 1)

##########
#@.......#
#..####..#
#........#
##########

A stranger flips an old coin and grins.
Call it: (h)eads or (t)ails? h
The coin spins...
It lands on heads. You called it! You win the duel.
```

Read that middle section carefully: `entity_move(player, 1, 0)` was
supposed to step the hero one tile east, and "Player position" prints
`(1, 1)` — completely unchanged. The map redraws with `@` in exactly the
same spot. **It didn't work, it compiled without a single warning, and
nothing told you it failed.**

## What just happened

Go back to the `struct` spotlight example: `Point moved = start; moved.x
+= 10;` never touched `start`, because assignment copies. Function
parameters work by that exact same rule — passing `player` into
`entity_move(Player p, ...)` copies the entire struct into `p`. Every
change `entity_move` makes is a change to that copy, which is discarded
the moment the function returns. `player` back in `main`, the one you
actually care about, was never touched.

```
main's player                    entity_move's p (a full copy)
+----------------+                +----------------+
| name: "Elowen" |                | name: "Elowen" |
| x: 1           |   copied in    | x: 1  -> 2     |  (changed HERE)
| y: 1           |  ------------> | y: 1           |
| hp: 20 ...     |                | hp: 20 ...     |
+----------------+                +----------------+
        ^                                  |
        |                        discarded when
   never touched                 entity_move returns
```

This is called **pass-by-value**: every ordinary function parameter in C
is a copy of whatever was passed in, full stop, whether it's an `int`, a
`Player`, or anything else you've seen so far. It's not a bug in
`entity_move` — it's C working exactly as specified. What's missing is a
way to say "don't give this function a copy — give it a way to reach back
and change the original." That's next chapter's entire subject, and it's
the single most important idea in the whole language.

## Common errors

**Forgetting the semicolon after a struct or typedef's closing brace:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -c missemi.c -o /dev/null
missemi.c:6:8: error: expected ‘;’ before ‘int’
    6 | } Point
      |        ^
      |        ;
missemi.c: In function ‘main’:
missemi.c:10:5: error: unknown type name ‘Point’; did you mean ‘int’?
   10 |     Point p = {1, 2};
      |     ^~~~~
      |     int
```

One missing semicolon after `} Point` cascades into a completely unrelated
-looking error a few lines later (`unknown type name 'Point'`) — because
the compiler, having failed to see the struct definition end properly,
never registered `Point` as a real type at all. **Always fix the first
error the compiler reports and recompile before worrying about the rest**
— later errors are frequently just confused aftermath of the first one,
not independent problems.

**Comparing two structs directly with `==`:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o structcmp structcmp.c
structcmp.c: In function ‘main’:
structcmp.c:12:11: error: invalid operands to binary == (have ‘Point’ and ‘Point’)
   12 |     if (a == b) {
      |           ^~
```

Unlike `int`, `char`, or `float`, C has no built-in notion of "these two
structs are equal" — `==` only works on individual values it knows how to
compare directly. Comparing two structs member-by-member (`a.x == b.x &&
a.y == b.y`) is something you write yourself, on purpose, when you need
it.

## Exercises

1. Add a `gold` field (`int`) to `Player`, initialize it to some starting
   amount in `entity_create_player`, and print it in `entity_print_sheet`.
2. Add a seventh rank, `RANK_SS`, above `RANK_S`. What integer value does
   it get automatically? Update `rank_to_char` to handle it — what
   happens if you forget to add a `case` for it and compile anyway? (Try
   it and read the warning, if any, before assuming.)
3. Without changing `entity_move` itself, could you make the position
   update "work" by changing how you *call* it — for example, reassigning
   `player = ...` using its return value? `entity_move` currently returns
   `void`. Sketch what its signature and body would need to look like for
   a return-value-based fix. (You don't need it to compile — Chapter 7
   covers a cleaner fix. This is about predicting the shape of a solution
   from what you already know.)
4. *Open-ended:* Real RPGs usually have more than one party member.
   Sketch (comments only, no working code required) what an `Npc` or
   second-party-member struct might share with `Player`, and what would
   need to differ.

<details>
<summary>Solutions</summary>

1. ```c
   /* entity.h, inside Player */
   int gold;
   ```
   ```c
   /* entity.c, in entity_create_player */
   p.gold = 50;
   ```
   ```c
   /* entity.c, in entity_print_sheet */
   printf("Gold:   %d\n", p.gold);
   ```

2. `RANK_SS` gets `5`, continuing the automatic count from `RANK_S`'s `4`.
   If you compile `rank_to_char`'s `switch` without adding a matching
   `case RANK_SS:`, `-Wall` (already part of this course's standard flags)
   catches it:
   ```
   swtest.c: In function ‘rank_to_char’:
   swtest.c:15:5: warning: enumeration value ‘RANK_SS’ not handled in switch [-Wswitch]
      15 |     switch (rank) {
         |     ^~~~~~
   ```
   `-Wswitch` specifically watches for a `switch` over an `enum` type that
   has no `default:` label and doesn't cover every enum value — exactly
   `rank_to_char`'s shape. It still compiles (this is a warning, not an
   error) and falls through to the function's final `return '?';`,
   printing a placeholder rank letter instead of a real one — but you get
   a compiler warning telling you exactly what to fix, for free, precisely
   because `rank_to_char` has no `default:` case. Adding one (to handle
   "shouldn't happen" cases deliberately) would silence this warning
   entirely, which is a real trade-off worth knowing: a `default:` is
   sometimes exactly right, and sometimes it's how a genuine gap like this
   one goes unnoticed.

3. The shape would be: change `entity_move`'s return type from `void` to
   `Player`, have it modify its local copy `p` same as before, and end
   with `return p;` — then call it as `player = entity_move(player, 1,
   0);` in `main`, reassigning `player` to the (moved) returned copy. This
   actually works, and is a completely legitimate way to solve this
   particular problem! It's just not the way this course is headed,
   because it doesn't scale — imagine a function that needs to modify
   *five* different values at once; returning all five bundled up gets
   awkward fast, and it's easy to forget to reassign the result at the
   call site. Chapter 7's fix solves this generally, for any number of
   values, without needing a return at all.

4. No fixed answer — a strong sketch would keep `name`, `x`, `y`, and the
   core stats in common (perhaps by literally reusing `Player`'s shape or
   even the type itself, depending on how identical NPCs and the hero
   turn out to be), while an NPC likely needs something a `Player`
   doesn't: dialogue text, or a flag for whether it can be talked to.
   You're not expected to have settled this — Chapter 13 introduces NPCs
   for real.

</details>

## Next up

Chapter 7 explains exactly why `entity_move` failed, using the concept
this whole chapter has been quietly building toward: pointers. It's the
crux of this course — budget some extra time for it — and by the end, your
hero will actually walk.
