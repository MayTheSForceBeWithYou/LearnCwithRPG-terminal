# Chapter 12: Strings

## Where we are

Chapter 11's map loader is full of string handling you took on faith:
`strlen` measuring a line, a loop trimming `\n` and `\r`, `memcpy` moving
exactly `width` bytes. It works, and you haven't been told *why* it works.
This chapter closes that gap — what a C string actually is, why it ends in
a NUL byte, and how the standard library's string functions manage to be
both essential and the most notorious source of security holes in the
language. Then you'll use all of it to make an NPC speak in a text box.

## The problem

C has no string type. It has arrays of `char` and a *convention*, and the
entire standard library depends on you upholding that convention perfectly
every time. Break it in one place and functions that seem unrelated start
reading past the end of your data. Getting text right is also the last
thing standing between your game and dialogue — and dialogue is what turns
a grid with an `@` on it into a world.

## C concept spotlight

### There is no string type — only a convention

```c
char greeting[] = "Hi";

printf("sizeof greeting = %zu\n", sizeof greeting);
printf("strlen(greeting) = %zu\n", strlen(greeting));

for (size_t i = 0; i < sizeof greeting; i++) {
    printf("  [%zu] = %3d  '%c'\n", i, greeting[i],
           greeting[i] == '\0' ? '0' : greeting[i]);
}
```

```
sizeof greeting = 3
strlen(greeting) = 2
  [0] =  72  'H'
  [1] = 105  'i'
  [2] =   0  '0'
```

Two characters, three bytes. That third byte — value `0`, written `'\0'`
and called the **NUL terminator** — is the entire convention. A "C string"
is just a `char` array that happens to contain a `0` byte marking where
the text stops.

This is why `sizeof` and `strlen` disagree, and the difference matters
constantly:

- `sizeof greeting` is **3** — the array's total size in bytes, known at
  compile time, including the terminator.
- `strlen(greeting)` is **2** — the number of characters *before* the
  terminator, counted at runtime by walking the array until it finds a `0`.

`strlen` genuinely walks the memory every time you call it. It has no
other way to know where the string ends. That has a consequence worth
internalizing early: **if the NUL is missing, `strlen` keeps walking** —
past the end of your array, through whatever memory follows, until it
happens across a zero byte or the program crashes. There's no length
stored anywhere for it to consult.

*Why like this?* It's a memory-efficiency decision from the early 1970s,
when a machine might have had 64 KB total. Storing a length prefix costs
extra bytes per string; a single terminator byte costs one, and lets a
string be any length. The tradeoff — every operation is O(n), and one
missing byte turns a bug into a memory-safety hole — is one the industry
has been paying interest on ever since.

### `char *` versus `char[]`

Two declarations that look similar and behave very differently:

```c
char  buffer[64] = "Hello";     /* an array: 64 writable bytes */
const char *text = "Hello";     /* a pointer to a string literal */
```

`buffer` is storage you own — 64 bytes on the stack, of which the first
six are used. You can modify it freely.

`text` points at a **string literal**, which lives in read-only memory
baked into your executable. Writing through it is undefined behaviour and
typically crashes. That's why this course writes `const char *` for
literals: the `const` makes the compiler enforce what the hardware would
otherwise enforce with a segfault. You saw this in Chapter 11 —
`map_load(const char *path)` — and in this chapter's `Npc.speech`.

### The dangerous functions

```c
char small[8];
const char *source = "This is far too long to fit";

strcpy(small, source);
```

`strcpy` copies until it hits the source's NUL. It has *no idea* how big
the destination is — you never told it, and there's no way to ask. Modern
GCC catches this particular case at compile time, because both sizes are
visible:

```
overflow.c: In function ‘main’:
overflow.c:9:5: warning: ‘strcpy’ writing 28 bytes into a region of size 8 overflows the destination [-Wstringop-overflow=]
    9 |     strcpy(small, source);
      |     ^~~~~~~~~~~~~~~~~~~~~
overflow.c:6:10: note: destination object ‘small’ of size 8
    6 |     char small[8];
      |          ^~~~~
```

But real overflows come from data the compiler can't see through — a name
loaded from a file, a value chosen at runtime:

```c
const char *load_npc_name(int which)
{
    static const char *names[] = {
        "Bo",
        "Archivist Perrin Culch of the Guild of Heraldic Reassembly"
    };
    return names[which % 2];
}

int main(int argc, char **argv)
{
    char name[8];
    const char *source = load_npc_name(argc);

    strcpy(name, source);
    printf("Hello, %s\n", name);
    return 0;
}
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o hidden hidden.c
$ ./hidden
*** stack smashing detected ***: terminated
```

**No compile-time warning at all** — the compiler can't know which name
comes back. What saved you is the stack protector from Chapter 5, catching
the damage after the fact. Under the sanitizer you get the real diagnosis:

```
==62718==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x76effa1d0028
WRITE of size 59 at 0x76effa1d0028 thread T0
    #1 0x64b3d22d23fd in main /tmp/ch12demo/hidden.c:19
```

59 bytes written into an 8-byte buffer. This exact shape — an unchecked
copy into a fixed buffer — is the single most exploited class of bug in
the history of C. Treat `strcpy`, `strcat`, and `sprintf` as functions
you simply do not use.

### The safe ones: `snprintf` and `memcpy`

`snprintf` formats text like `printf`, but into a buffer whose size you
state, and it *always* terminates:

```c
char buf[10];
int needed = snprintf(buf, sizeof buf, "HP %d/%d  MP %d", 18, 20, 8);
```

```
buf     = |HP 18/20 |
needed  = 14
-> TRUNCATED (needed 14, had room for 9)
```

The return value is the length the string **would have needed**, not what
it wrote. So `needed >= sizeof buf` is how you detect truncation — and
truncation is silent otherwise. Always pass `sizeof buf`, never a
hardcoded number that can drift out of sync with the declaration.

`memcpy` copies an exact byte count with no interest in NUL bytes at all —
which is precisely why Chapter 11's map loader used it. Map rows aren't
strings; they're exactly `width` bytes of tile data, and stopping early at
a `0` would have been wrong.

One to be wary of: **`strncpy` is not the safe `strcpy`.** If the source
doesn't fit, it copies exactly `n` bytes and *does not terminate*:

```c
char dst[8];
strncpy(dst, "abcdefghij", sizeof dst);
printf("last byte is NUL? %s\n", dst[7] == '\0' ? "yes" : "NO");
```

```
last byte is NUL? NO
dst[7] = 104 ('h')
```

You now hold an unterminated `char` array that every string function will
happily read past the end of. `strncpy` was designed in 1975 for a
fixed-width filesystem field format, not for safe copying, and its name is
a historical accident. Use `snprintf` instead.

### Word wrapping

Wrapping a paragraph to a width is: scan forward up to `width` characters;
if you landed mid-word, back up to the last space; emit that slice; skip
the space; repeat. The subtleties are all in the edge cases — a word
longer than the line, trailing spaces, running out of lines — which is
exactly why it's worth writing once, carefully, in its own module.

## Apply it

### The dialog module

`dialog.h`:

```c
#ifndef DIALOG_H
#define DIALOG_H

#define DIALOG_MAX_LINES 4
#define DIALOG_LINE_LEN  48

typedef struct {
    char lines[DIALOG_MAX_LINES][DIALOG_LINE_LEN];
    int line_count;
} Dialog;

/* Word-wraps text into dialog->lines, breaking at spaces where possible.
   Never writes more than DIALOG_LINE_LEN - 1 characters plus a NUL into
   any line, and never fills more than DIALOG_MAX_LINES lines; text that
   does not fit is dropped. width is clamped to the line capacity. */
void dialog_wrap(Dialog *dialog, const char *text, int width);

#endif
```

The comment states the guarantees precisely, including the unflattering
one — text beyond four lines is **dropped**. Saying so is better than a
caller discovering it. (Fixing it properly means paging, which is an
exercise.)

`dialog.c`:

```c
#include <string.h>
#include "dialog.h"

void dialog_wrap(Dialog *dialog, const char *text, int width)
{
    dialog->line_count = 0;

    if (text == NULL) {
        return;
    }

    /* Never allow a caller to ask for more than a line can hold. */
    if (width > DIALOG_LINE_LEN - 1) {
        width = DIALOG_LINE_LEN - 1;
    }
    if (width < 1) {
        width = 1;
    }

    size_t pos = 0;
    size_t len = strlen(text);

    while (pos < len && dialog->line_count < DIALOG_MAX_LINES) {
        /* Skip any spaces that would otherwise start a line. */
        while (text[pos] == ' ') {
            pos++;
        }
        if (pos >= len) {
            break;
        }

        /* How much of the remaining text could fit on one line? */
        size_t remaining = len - pos;
        size_t take = remaining < (size_t)width ? remaining : (size_t)width;

        /* If we are cutting mid-word, back up to the last space. */
        if (take < remaining && text[pos + take] != ' ') {
            size_t back = take;
            while (back > 0 && text[pos + back - 1] != ' ') {
                back--;
            }
            /* back == 0 means a single word longer than the line, which
               we have to split rather than loop forever. */
            if (back > 0) {
                take = back;
            }
        }

        /* Drop the trailing space, if the break landed on one. */
        size_t copy = take;
        while (copy > 0 && text[pos + copy - 1] == ' ') {
            copy--;
        }

        memcpy(dialog->lines[dialog->line_count], text + pos, copy);
        dialog->lines[dialog->line_count][copy] = '\0';
        dialog->line_count++;

        pos += take;
    }
}
```

Every line of defensive code here exists because of a specific failure:

- **Clamping `width`** to `DIALOG_LINE_LEN - 1` means a caller passing a
  huge width can't overflow a line. The module protects itself rather
  than trusting its caller — the same instinct as Chapter 11's loader.
- **The `back > 0` check** prevents an infinite loop. A word longer than
  the whole line has no space to back up to; without this, `take` would
  become `0`, `pos` would never advance, and the game would hang. That's
  the Chapter 3 infinite-loop bug hiding inside a text algorithm.
- **Writing the NUL explicitly** after `memcpy` is what makes each line a
  real string. `memcpy` doesn't add one; skipping this leaves
  `render_draw_text` reading off the end.
- **`width < 1`** guards against a zero or negative width producing a
  zero-length slice and, again, an infinite loop.

### An NPC to talk to

Add to `entity.h`, above the function declarations:

```c
typedef struct {
    int x;
    int y;
    const char *speech;
} Npc;
```

One NPC, one struct — a *roster* of NPCs is an array of structs, which is
Chapter 13's job.

Add `TILE_NPC` to `render.h`'s `TileType`, and draw it in
`render_ncurses.c`:

```c
case TILE_NPC:    symbol = 'o'; break;
```

### Wiring it into the game

In `main.c`, create the NPC and wrap its speech once at startup:

```c
#define TEXTBOX_WIDTH (VIEW_WIDTH - 2)
```

```c
Npc gatekeeper = {
    .x = 4,
    .y = 1,
    .speech = "Careful out there. The roads have not been "
              "safe for a long while now."
};

Dialog dialog;
dialog_wrap(&dialog, gatekeeper.speech, TEXTBOX_WIDTH);
```

Two things worth noting. The `.x = 4` syntax is a **designated
initializer** — naming members explicitly instead of relying on
declaration order, which is both clearer and immune to breaking if someone
reorders the struct. And the two adjacent string literals on separate
lines are automatically concatenated by the compiler into one string,
which is how you write long text without absurdly long source lines.

Draw the text box when talking:

```c
static void draw_textbox(const Dialog *dialog)
{
    int top = VIEW_HEIGHT + 1;

    for (int i = 0; i < dialog->line_count; i++) {
        render_draw_text(1, top + i, dialog->lines[i]);
    }
}
```

And make walking into the NPC start a conversation instead of a move:

```c
if (dx != 0 || dy != 0) {
    int target_x = player.x + dx;
    int target_y = player.y + dy;

    if (target_x == gatekeeper.x && target_y == gatekeeper.y) {
        /* Walking into someone starts a conversation instead. */
        talking = 1;
    } else if (map_is_walkable(map, target_x, target_y)) {
        talking = 0;
        entity_move(&player, dx, dy);
    }
}
```

Add `dialog.c` to `SOURCES`, and note the NPC must stand on a floor tile —
`(4, 1)` is inside the starting room.

## Compile and run

```bash
make clean
make
./game
```

Zero warnings. You'll see the gatekeeper as an `o` a few tiles east:

```
####################
#@..o..#############
#......#############
#......#####........
#............#######
#......#####........
#......#############
#......#############
#...................
############...#####

w/a/s/d to move, q to quit wandering
```

Walk into them (`d` three times) and the text box appears, wrapped to
width 18 with no word split across lines:

```
####################
#..@o..#############
#......#############
#......#####........
#............#######
#......#####........
#......#############
#......#############
#...................
############...#####

 Careful out there.
 The roads have not
 been safe for a
 long while now.
```

And the sanitizer check, required from Chapter 10 onward:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

Silence. Text handling is where buffer bugs live, so this run matters more
than most.

## What just happened

`dialog_wrap` walked the speech once, slicing it into four NUL-terminated
lines inside the `Dialog` struct. No allocation was involved — the lines
are fixed-size arrays *inside* the struct, so the whole thing lives on the
stack in `main` and dies when `main` returns. That's a deliberate choice:
bounded text with a known maximum doesn't need the heap, and not using the
heap means nothing to leak.

```
  gatekeeper.speech (a string literal, read-only)
  "Careful out there. The roads have not been safe for a long while now."
   |------- 18 -------|
                       ^ back up to the last space, emit the slice

  Dialog (on the stack, 4 x 48 bytes)
  +--------------------------------------------------+
  | lines[0] | "Careful out there." \0 . . . unused   |
  | lines[1] | "The roads have not"  \0 . . . unused  |
  | lines[2] | "been safe for a"     \0 . . . unused  |
  | lines[3] | "long while now."     \0 . . . unused  |
  | line_count = 4                                    |
  +--------------------------------------------------+
```

Every line ends in a `\0` your code wrote deliberately, which is the only
reason `render_draw_text` — which takes a `const char *` and trusts the
convention — stops in the right place.

## Common errors

**Off-by-one: forgetting the terminator needs a byte:**

```c
char name[5];
snprintf(name, sizeof name, "Elowen");
```

No warning, no crash — but `name` holds `"Elow"`, silently truncated,
because 5 bytes means 4 characters plus the NUL. Whenever you size a
buffer, size it for the longest text *plus one*.

**Using `strlen` on unterminated data:**

```c
char row[40];
memcpy(row, source, 40);      /* exactly 40 bytes, no NUL */
size_t n = strlen(row);       /* walks past the end */
```

```
==63104==ERROR: AddressSanitizer: stack-buffer-overflow
READ of size 1 at 0x7ffd... thread T0
```

This is the Chapter 11 map row, and it's why the loader used `memcpy` into
a sized tile block rather than treating rows as strings. Raw byte data and
strings are different things; mixing them up is how NULs go missing.

**`snprintf` truncation ignored:**

```
snp.c:6:55: warning: ‘  MP ’ directive output truncated writing 5 bytes into a region of size 2 [-Wformat-truncation=]
    6 |     int needed = snprintf(buf, sizeof buf, "HP %d/%d  MP %d", 18, 20, 8);
      |                                                     ~~^~~
snp.c:6:18: note: ‘snprintf’ output 15 bytes into a destination of size 10
```

`-Wformat-truncation` catches the cases it can prove. When the values are
runtime data it can't, and the truncation is silent — check the return
value when it matters.

## Exercises

1. Change `DIALOG_LINE_LEN` to `12` but leave `TEXTBOX_WIDTH` alone.
   What happens to the text box, and why doesn't it overflow? (Trace which
   clamp in `dialog_wrap` saves you.)
2. Give the gatekeeper a much longer speech — five or six sentences.
   What happens to the text beyond four lines? Is the failure visible to
   the player, and is that acceptable?
3. Implement paging: add a `dialog_next_page` function so long speeches
   show four lines at a time and advance when the player presses a key.
   (Hint: `dialog_wrap` currently starts at `pos = 0` every time; you'll
   need to remember where the previous page stopped.)
4. *Open-ended:* NPC speech is currently a string literal compiled into
   the program — exactly the problem Chapter 11 solved for maps. Sketch a
   file format for dialogue. What has to go in it besides the text itself,
   if an NPC is eventually going to say different things depending on what
   you've done?

<details>
<summary>Solutions</summary>

1. `TEXTBOX_WIDTH` is `VIEW_WIDTH - 2` = 18, but `DIALOG_LINE_LEN - 1` is
   now 11, so the first clamp in `dialog_wrap` reduces `width` to 11.
   Lines wrap much more narrowly than the box allows — visually worse,
   but perfectly safe. That clamp is doing exactly the job it was written
   for: the module refuses to write more than its own buffers hold,
   regardless of what the caller asks for. Had it trusted `width`, this
   change would have written 18 bytes into 12-byte lines.

2. Everything past the fourth line is silently dropped — the player sees a
   speech that stops mid-thought with no indication there was more. It's
   not acceptable for real dialogue, which is why exercise 3 exists. It
   *is* acceptable as an intermediate state, as long as it's documented
   (which `dialog.h`'s comment does) rather than discovered later.

3. The core change is tracking a read position across calls:
   ```c
   typedef struct {
       char lines[DIALOG_MAX_LINES][DIALOG_LINE_LEN];
       int line_count;
       const char *source;
       size_t pos;          /* where the next page starts */
       int width;
   } Dialog;

   void dialog_start(Dialog *d, const char *text, int width);
   int  dialog_next_page(Dialog *d);   /* 1 if a page was produced */
   ```
   `dialog_start` stores `source`, `width`, and sets `pos = 0`;
   `dialog_next_page` runs the existing wrap loop starting from `d->pos`,
   leaves `pos` where it stopped, and returns `0` when there's nothing
   left. `main` then calls `dialog_next_page` on each keypress while
   talking, and ends the conversation when it returns `0`. Note the
   `Dialog` now holds a pointer to text it does not own — worth a comment
   saying the caller must keep that text alive, which for a string literal
   is automatic.

4. No fixed answer. Beyond the text, a dialogue file needs at minimum an
   **identifier** per entry so an NPC can reference one, and to support
   conditional speech it needs some notion of **state** — a flag or quest
   step the line depends on. A plausible sketch:
   ```
   [gatekeeper.default]
   Careful out there. The roads have not been safe for a long while now.

   [gatekeeper.after_first_fragment]
   You found one? Then the stories were true after all.
   ```
   That bracketed-section shape is essentially INI, and parsing it is
   Chapter 21's territory. The real design question you're being nudged
   toward: who decides which entry is current — the NPC, or the game
   state? (Usually the game state, which is why Chapter 14's state
   machines come first.)

</details>

## Next up

You have one hard-coded NPC standing at one hard-coded position, and a
single map with no way out of it. **Checkpoint B comes next** — before
Chapter 13, you'll be asked what your world actually *is*: its setting,
its tone, what went wrong in it, and whether you travel alone or with a
party. Those answers shape every chapter that follows, so it's worth
thinking about before you turn the page.
