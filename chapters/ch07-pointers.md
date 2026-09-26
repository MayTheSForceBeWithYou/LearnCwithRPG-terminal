# Chapter 7: Pointers

## Where we are

Chapter 6 ended with a genuine failure: `entity_move` looked correct,
compiled without one single warning, and did nothing at all. `player.x`
never changed, because `entity_move(Player p, ...)` only ever touched a
copy. This chapter is the payoff for that failure — and it's the most
important chapter in this course. Budget real time for it. By the end,
`entity_move` actually works, your hero walks around the map, and walls
stop them.

## The problem

You need a way to hand a function not a *copy* of your data, but a way to
reach back and change the *original* — for `entity_move`, for anything
else you'll build from here on. C's answer is the pointer: a value that
doesn't hold data directly, but holds *where* some data lives.

## C concept spotlight

### Every variable lives somewhere: `&`

Every variable in your program occupies real memory, at a real address —
you just haven't had a reason to look until now.

```c
int hp = 20;
printf("hp itself:        %d\n", hp);
printf("hp lives at:      %p\n", (void *)&hp);
```

```
hp itself:        20
hp lives at:      0x7ffd7d594294
```

`&hp` is the **address-of operator** — it doesn't compute anything about
`hp`'s *value*, it asks "where does `hp` live?" and returns that address.
`%p` is the format specifier for printing addresses, and it specifically
expects a `void *` — a generic pointer type meaning "an address, type
unspecified" — which is why the cast `(void *)&hp` appears; without it,
`gcc` would warn about a format mismatch, the exact same family of [warning
from Chapter 2](ch02-data-and-input.md#a-mismatched-format-specifier--caught-by-the-compiler). Your address will differ from the one above every time you
run the program — the operating system decides where things live, and
that decision isn't something your code controls or should rely on.

### Pointers: a variable that holds an address

```c
int hp = 20;
int *hp_ptr = &hp;

printf("hp_ptr points at: %p\n", (void *)hp_ptr);
printf("value there:      %d\n", *hp_ptr);

*hp_ptr = 5;
printf("hp is now:        %d\n", hp);
```

```
hp_ptr points at: 0x7ffd84f820cc
value there:      20
hp is now:        5
```

`int *hp_ptr` declares `hp_ptr` as a **pointer to `int`** — a variable
whose value is an address, specifically the address of some `int`
somewhere. `hp_ptr = &hp` makes it point at `hp`. The `*` shows up in two
different jobs here, and conflating them is the single most common
pointer confusion beginners hit:

- In a **declaration** (`int *hp_ptr`), `*` means "this variable is a
  pointer."
- In an **expression** (`*hp_ptr = 5`), `*` is the **dereference
  operator** — "go to the address this pointer holds, and access the
  value there." `*hp_ptr = 5` doesn't change `hp_ptr` itself (it still
  points at `hp`); it changes *what `hp_ptr` points at* — `hp`.

```
Stack memory (addresses made up for illustration):

  0x7ffd...294  +--------------+
  hp            |      20      |  <-- *hp_ptr = 5 changes THIS box
                +--------------+        to 5, not hp_ptr itself
  0x7ffd...0cc  +--------------+
  hp_ptr        | 0x7ffd...294 |  <-- hp_ptr's own value: an address
                +--------------+
```

### Pass-by-reference: fixing `entity_move`, in miniature

Here's the exact bug from Chapter 6, and its fix, distilled to five lines:

```c
void increment(int *n)
{
    *n += 1;
}

int main(void)
{
    int hp = 20;
    increment(&hp);
    printf("hp = %d\n", hp);   /* 21 */
    return 0;
}
```

`increment` doesn't receive a copy of `hp` — it receives `hp`'s *address*,
and `*n += 1` reaches through that address to modify `hp` directly. This
is **pass-by-reference**: instead of passing a value, you pass a pointer
to where that value lives, so the function can change the caller's actual
data. Compare this directly to Chapter 6's broken `entity_move(Player p,
...)`, which took `Player p` by value — a full copy, exactly like passing
`int n` by value instead of `int *n` here. Same bug, same fix, just at
different scale.

### `NULL`: a pointer that points at nothing

A pointer doesn't have to point at a real variable — it can explicitly
point at nothing, using the special value `NULL`:

```c
int *p = NULL;
printf("%d\n", *p);
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o nullderef nullderef.c
$ ./nullderef
[nothing printed]
$ echo $?
139
```

Exit status 139 — the same segmentation fault from Chapter 2's mismatched
`printf`. Dereferencing `NULL` is undefined behaviour, and in practice it
almost always crashes immediately, because `NULL` is guaranteed to never
be a valid address for real data — the operating system reserves it
specifically so that accessing it fails loudly rather than silently
corrupting something. `NULL` is genuinely useful: it's how a pointer says
"I deliberately don't have a value yet" (as opposed to some garbage
leftover address, which is far more dangerous because it *looks* valid).
You won't create a `NULL` pointer on purpose much in this chapter, but
you'll see the pattern "check if a pointer is `NULL` before using it"
constantly from here on, especially once Chapter 10 introduces pointers
that can fail to point at anything valid at all.

### `const`-correctness: promising not to change what you point at

Passing a pointer means "here's a way to reach my data" — sometimes you
want to grant read access without granting write access:

```c
void print_value(const int *n)
{
    printf("%d\n", *n);
    *n = 99;   /* now try to compile this */
}
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o constdemo constdemo.c
constdemo.c: In function ‘print_value’:
constdemo.c:6:8: error: assignment of read-only location ‘*n’
    6 |     *n = 99;
      |        ^
```

`const int *n` reads as "a pointer to an `int` I promise not to modify
through this pointer." That promise is enforced by the compiler, at
compile time, for free — anyone reading `print_value`'s signature knows
immediately it only looks at `*n`, never changes it, without reading a
single line of the function body. Use `const` on every pointer parameter a
function doesn't need to modify; it costs nothing and documents intent
precisely.

### `->`: reaching into a struct through a pointer

Chapter 6 accessed struct members with `.` — `p.x`. Once you have a
*pointer* to a struct instead of the struct itself, `.` doesn't work
directly (you'd have to write `(*p).x`, dereference-then-access, which is
clunky enough that C gives you a shorthand):

```c
typedef struct {
    int x;
    int y;
} Point;

void bump(Point *p)
{
    p->x += 1;   /* shorthand for (*p).x += 1 */
}

int main(void)
{
    Point pos = {3, 4};
    bump(&pos);
    printf("pos.x = %d\n", pos.x);   /* 4 */
    return 0;
}
```

`p->x` means "follow the pointer `p`, then access member `x`." You'll use
`->` constantly from here on, anywhere a struct is reached through a
pointer instead of held directly.

## Apply it

Fix `entity_move` for real. In `entity.h`, change its signature to take a
pointer:

```c
void entity_move(Player *p, int dx, int dy);
```

And in `entity.c`:

```c
void entity_move(Player *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}
```

While you're in `entity.h`, apply `const`-correctness to
`entity_print_sheet` too — it only reads a `Player`, so pass a pointer it
promises not to modify, avoiding an unnecessary full-struct copy in the
process:

```c
void entity_print_sheet(const Player *p);
```

```c
void entity_print_sheet(const Player *p)
{
    float power_rating = (float)(p->attack + p->defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", p->name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", p->hp);
    printf("MP:     %d\n", p->mp);
    printf("ATK:    %d\n", p->attack);
    printf("DEF:    %d\n", p->defense);
    printf("Rank:   %c\n", rank_to_char(p->rank));
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}
```

Now give the map a way to answer "can the player stand here?" Add to
`map.h`:

```c
int map_is_walkable(int x, int y);
```

And to `map.c`:

```c
int map_is_walkable(int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 0;
    }

    return world[y][x] != '#';
}
```

The bounds check matters even though this particular map's outer wall
should always stop the player first — Chapter 5 spent an entire "Common
errors" section on exactly what happens when you skip a check like this
and trust the data instead. Free-standing, defensive bounds checks are a
habit worth having before you need them, not after.

Finally, replace `main.c`'s single scripted "step east" demo from Chapter
6 with a real, repeatable movement loop:

```c
static char read_command(void)
{
    printf("Move (w/a/s/d, q to stop wandering): ");

    int typed = getchar();
    int discard = typed;

    while (discard != '\n' && discard != EOF) {
        discard = getchar();
    }

    return (char)typed;
}
```

```c
Player player = entity_create_player();
entity_print_sheet(&player);

printf("\n");
printf("The village gate creaks open.\n");
printf("\n");

char command = '\0';
while (command != 'q') {
    map_print(player.x, player.y);

    command = read_command();

    int dx = 0;
    int dy = 0;

    switch (command) {
        case 'w': dy = -1; break;
        case 's': dy = 1;  break;
        case 'a': dx = -1; break;
        case 'd': dx = 1;  break;
        case 'q': break;
        default:
            printf("Not a direction I know. Try w/a/s/d/q.\n");
            break;
    }

    if (dx != 0 || dy != 0) {
        int target_x = player.x + dx;
        int target_y = player.y + dy;

        if (map_is_walkable(target_x, target_y)) {
            entity_move(&player, dx, dy);
        } else {
            printf("You bump into a wall.\n");
        }
    }

    printf("\n");
}
```

Note `entity_print_sheet(&player)` — passing `&player` because
`entity_print_sheet` now takes a pointer — and `entity_move(&player, dx,
dy)`, same reason, this time actually mutating `player` for real. You'll
have to press Enter after every move; that's clunky, and it's honest about
where this course is — Chapter 8 replaces this entire input loop with
something that reacts the instant you press a key, no Enter required.

## Compile and run

```bash
make clean
make
./game
```

Zero warnings. Try moving around — `d` to step east onto open floor,
`w`/`s` toward the wall segment in the middle of the room to confirm it
actually blocks you, `q` to stop wandering and move on to the duel.
Expected shape of the output, moving east twice then bumping a wall to the
south:

```
##########
#@.......#
#..####..#
#........#
##########
Move (w/a/s/d, q to stop wandering): d

##########
#.@......#
#..####..#
#........#
##########
Move (w/a/s/d, q to stop wandering): d

##########
#..@.....#
#..####..#
#........#
##########
Move (w/a/s/d, q to stop wandering): s
You bump into a wall.

##########
#..@.....#
#..####..#
#........#
##########
Move (w/a/s/d, q to stop wandering): q
```

## What just happened

`entity_move(&player, dx, dy)` passes `player`'s *address* into `p`.
Inside `entity_move`, `p->x += dx` follows that address back to the exact
same memory `main`'s `player` occupies and modifies it there — no copy,
no discard-on-return, because there was never a second copy to discard.

```
main's player                entity_move's p holds player's address
+----------------+                 (a pointer, not a copy)
| name: "Elowen" |  <------------------+
| x: 1  -> 2     |  <--------- p->x += dx reaches all the way back here
| y: 1           |   |
| hp: 20 ...     |   |
+----------------+   |
        ^            |
        |  p         |
        +------------+
```

This is the fix Chapter 6 promised: not "return the new value and
remember to reassign" (which works, but doesn't scale past one value, as
that chapter's exercises showed) but "hand the function a way to reach the
original directly." Every mutation you write from here forward — battle
damage, inventory changes, save data — follows this exact same shape:
pointer in, real change out.

## Common errors

**Forgetting `&` when calling a pointer-parameter function:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -c badcall.c -o /dev/null
badcall.c: In function ‘main’:
badcall.c:16:17: error: incompatible type for argument 1 of ‘entity_move’
   16 |     entity_move(player, 1, 0);
      |                 ^~~~~~
      |                 |
      |                 Player
badcall.c:7:26: note: expected ‘Player *’ but argument is of type ‘Player’
    7 | void entity_move(Player *p, int dx, int dy)
      |                  ~~~~~~~~^
```

Forgetting `&` passes the *struct itself* where a *pointer* was expected —
a category error the compiler always catches, because a `Player` and a
`Player *` are visibly different types, and this one is a hard error, not
just a warning. This is one of the friendlier pointer mistakes precisely
because it's always caught at compile time, not silently wrong at
runtime.

**Dereferencing an uninitialized pointer:**

```c
int *p;
printf("%d\n", *p);
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o uninit uninit.c
uninit.c: In function ‘main’:
uninit.c:6:5: warning: ‘p’ is used uninitialized [-Wuninitialized]
    6 |     printf("%d\n", *p);
      |     ^~~~~~~~~~~~~~~~~~
uninit.c:5:10: note: ‘p’ was declared here
    5 |     int *p;
      |          ^
```

Unlike `NULL`, an uninitialized pointer doesn't reliably crash — it holds
whatever garbage address happened to already be sitting in that memory,
which might be an address your program has no business touching (usually
a crash) or, worse, might accidentally look like a valid address and let
you silently read or corrupt memory you didn't mean to touch. `-Wall`
catches the *obvious* cases of this, like here, but not all of them —
never rely on the warning; always initialize a pointer, either to a real
address or explicitly to `NULL`.

## Exercises

1. In `read_command`, what would happen if you removed the `discard` loop
   entirely (just `return (char)typed;` right after the first
   `getchar()`)? Trace through what happens on the *second* call to
   `read_command()` if the player typed `dd` (two characters) before
   pressing Enter on the first move. Then try it for real.
2. `map_is_walkable` takes plain `int x, int y`, not pointers — why is
   that fine here, when `entity_move` needed a pointer to actually change
   something? (Hint: what does `map_is_walkable` do with `x` and `y` —
   does it need to modify the caller's values, or just read them?)
3. Write a `swap` function: `void swap(int *a, int *b)` that exchanges the
   values two pointers point at. Test it with two `int` variables in
   `main`, confirming both actually swap. (This is one of the single most
   common pointer exercises in any C course, for good reason — if this
   one clicks, the whole chapter has clicked.)
4. *Open-ended:* `entity_move` currently has no way to say "that move was
   invalid" — the caller (`main`) checks `map_is_walkable` first and only
   calls `entity_move` if it's safe. Could `entity_move` itself return
   something indicating success or failure, so callers don't have to
   remember to check first? Sketch the signature (you don't need to
   implement it) — what type would the return value need, and does this
   change what `dx`/`dy` mean if the move gets rejected partway?

<details>
<summary>Solutions</summary>

1. Without the discard loop, the leftover `d` (from typing `dd` and
   pressing Enter) sits unread in the input stream. The *next* call to
   `read_command()` would immediately read that leftover `d` with its
   first `getchar()` — silently consuming a "move" the player never
   actually intended to make on that turn — before the newline is finally
   consumed on some later call, at which point everything gets
   desynchronized from what the player thinks they've typed. This is the
   exact same leftover-input problem Chapter 3 introduced with `ask_call`,
   now costing you actual gameplay correctness instead of just a
   re-prompt.

2. `map_is_walkable` only ever *reads* `x` and `y` to compute a yes/no
   answer — it never needs to change what the caller's coordinates are.
   Pointers exist specifically for when a function needs to modify the
   caller's data (or, later, avoid copying something large); passing
   plain `int`s by value is completely correct, and arguably clearer,
   whenever a function only needs to look at a value, never change it.
   Reaching for a pointer when a plain value would do is a common
   over-correction once pointers start to click — resist it.

3. ```c
   void swap(int *a, int *b)
   {
       int temp = *a;
       *a = *b;
       *b = temp;
   }

   int main(void)
   {
       int x = 1, y = 2;
       swap(&x, &y);
       printf("x = %d, y = %d\n", x, y);   /* x = 2, y = 1 */
       return 0;
   }
   ```
   The `temp` variable is essential — without it, `*a = *b;` would
   overwrite `a`'s original value before you had a chance to give it to
   `b`, losing it permanently. This three-line dance (save, overwrite,
   restore-from-save) is a pattern you'll reuse constantly.

4. A reasonable sketch: `int entity_move(Player *p, int dx, int dy)`,
   returning `1` for a successful move and `0` for a rejected one — but
   this immediately raises the question you're meant to notice: for
   `entity_move` to *know* whether a move is valid, it would need to ask
   the map itself, which means `entity.c` would need to `#include
   "map.h"` and know about map internals it currently doesn't. That's a
   real design tradeoff (a module boundary question, not a pointers
   question) — there's no single right answer yet, and you're not
   expected to resolve it now. Keeping the check in `main` for now, as
   this chapter did, is a legitimate choice, not a shortcut you'll
   necessarily regret.

</details>

## Next up

Movement currently requires pressing Enter after every single step, and the
whole game is one long top-to-bottom script with no real "loop" governing
input, drawing, and logic. Chapter 8 introduces a proper game loop and a
renderer abstraction — the architecture that makes raw, instant key input
possible, and the same abstraction that would let this project migrate to
SDL2 later without a rewrite.
