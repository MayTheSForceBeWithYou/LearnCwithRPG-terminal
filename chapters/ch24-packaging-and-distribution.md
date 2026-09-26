# Chapter 24: Packaging and Distribution

## Where we are

The game is finished. Twenty-three chapters, six locations, three bosses,
four crown fragments, an ending, and a test suite that runs three hundred
thousand assertions without complaint.

It also runs in exactly one place: the directory you built it in. Move the
binary anywhere else and it dies before the title screen finishes.

## The problem

Try it. From the project directory:

```bash
make
cp game /tmp
cd /tmp
./game
```

```
map_load: cannot open 'assets/maps/grubbin_vale.map'
world_create: could not load area 'Grubbin Vale'
Could not start the game. See the message above.
```

Every path in this program is relative: `assets/levels.txt`,
`assets/maps/mudwick.map`. Relative to *the working directory*, which for
twenty-three chapters has always happened to be the right one.

An installed program does not get that luxury. A player types `game` and
their shell is wherever they last `cd`'d to. So this chapter answers two
questions the game has never had to ask:

- **Where are my data files?** They ship with the program, and they are
  read-only.
- **Where do I put the save?** Somewhere the player can write, which is
  emphatically not `/usr/local/share`.

Those get different answers, and conflating them is the single most common
packaging mistake.

## C concept spotlight

### What `-lncursesw` actually does

You have typed it in every `Makefile` since Chapter 8 without asking what it
attaches to your program. Now is the time, because it is the reason a binary
that runs on your machine may not run on your friend's.

Compilation turns each `.c` into a `.o` full of machine code with holes in
it: every call to `initscr` is a *reference to a symbol that does not exist
yet*. **Linking** fills the holes. There are two ways to do it.

**Static linking** copies the library's code into your executable. The
holes are filled at build time, the result is self-contained, and it never
changes again.

**Dynamic linking** records a *promise* instead: "this program needs
`libncursesw.so.6`, and when you run it, go and find one." The filling-in
happens at program start, performed by the dynamic loader — that
`ld-linux-x86-64.so.2` you met in the Chapter 22 Valgrind troubles.

`-lncursesw` gives you the second, because that is the default. Ask any
binary what promises it made:

```bash
ldd ./game
```

```
	linux-vdso.so.1 (0x00007ffcf3faa000)
	libncursesw.so.6 => /usr/lib/libncursesw.so.6 (0x00007789f1606000)
	libc.so.6 => /usr/lib/libc.so.6 (0x00007789f1200000)
	/lib64/ld-linux-x86-64.so.2 => /usr/lib64/ld-linux-x86-64.so.2 (...)
```

Read it as a list of things that must exist on the machine that runs this,
and the paths where they were found *on this machine, just now*. Give the
binary to somebody without `libncursesw.so.6` and they get:

```
./game: error while loading shared libraries: libncursesw.so.6:
cannot open shared object file: No such file or directory
```

That message is the loader, not your program. Your `main` never ran.

`linux-vdso` is not a real file — it is a small library the kernel maps
into every process. There is nothing to install and nothing to worry about.

### Which to choose

| | Dynamic (default) | Static (`-static`) |
|---|---|---|
| Binary size | small | large — the library comes too |
| Security fixes | arrive with the system's library update | require rebuilding your program |
| Runs on other machines | only where the library exists | far more portable |
| Memory across processes | one shared copy | a copy per process |

For a game you hand to a friend running the same distribution, dynamic is
correct: it is smaller, and a bug fixed in ncurses is fixed for you without
you doing anything. Static is for shipping to machines you do not control.

The honest answer for this project is **dynamic, plus telling people what to
install** — one line in a README beats an eight-megabyte binary. On Arch:

```bash
sudo pacman -S --needed ncurses
```

You can see the static version's cost for yourself:

```bash
gcc -std=c17 -Wall -Wextra -g *.c -o game-static -static -lncursesw -ltinfow
```

It may not even link, depending on which static libraries your system has
installed — which is itself part of the answer.

### Compile-time configuration with `-D`

The other half of packaging is telling the program where its data went. You
cannot ask at runtime — there is nobody to ask. So you tell it at compile
time, with a macro defined on the command line:

```bash
gcc -DDATADIR='"/usr/local/share/some-assembly-required"' ...
```

`-DNAME=VALUE` defines a preprocessor macro exactly as if the file had
started with `#define NAME VALUE`. You have used its plainest form already:
`-DNDEBUG` in Chapter 22, which is just `#define NDEBUG` and makes every
`assert` vanish.

The quoting deserves a moment, because it catches everybody. The macro must
expand to a *C string literal*, so the double quotes have to survive the
shell:

```make
-DDATADIR='"$(DATADIR)"'
```

Single quotes protect the double quotes from the shell; the double quotes
make it a string in C. Get it wrong and you see:

```
<command-line>: error: expected expression before ‘/’ token
```

...because `/usr/local/share` arrived as bare tokens and the compiler tried
to divide by `usr`.

Code that uses such a macro should always provide a fallback, so a file
compiled by hand still builds:

```c
#ifndef DATADIR
#define DATADIR "/usr/local/share/some-assembly-required"
#endif
```


### `DESTDIR` stages; `PREFIX` ships

Staging under `/tmp/stage` must not be compiled into the binary. Practice
`03_destdir_story` makes the string story obvious before `make install`.


## Apply it: finding the data

Create `paths.h`. The comment matters more than the declarations:

```c
/* Two different questions, with two different answers:

     - Data files are READ-ONLY and shipped with the game. They may sit
       in the build tree, or in /usr/local/share once installed.
     - The save file is WRITTEN, so it cannot live beside the data at
       all: /usr/local/share is not writable by a player. It belongs in
       the user's own directory. */

void paths_data(char *out, size_t out_size, const char *relative);
int  paths_save(char *out, size_t out_size);
const char *paths_data_dir(void);
```

Both write into a caller-supplied buffer. A function that returns a pointer
to its own `static` buffer is easier to call and a menace to maintain: two
calls in one `printf` and the first result is gone. Chapter 12's rule about
who owns a buffer has not stopped applying.

`paths.c` resolves the data directory once, in order of specificity:

```c
static void resolve_data_dir(void)
{
    const char *override = getenv("SAR_DATA_DIR");

    if (override != NULL && override[0] != '\0') {
        snprintf(data_dir, sizeof data_dir, "%s", override);
    } else if (directory_exists("assets")) {
        snprintf(data_dir, sizeof data_dir, "assets");
    } else {
        snprintf(data_dir, sizeof data_dir, "%s", DATADIR);
    }

    data_dir_ready = 1;
}
```

Three cases, each earning its place:

1. **`$SAR_DATA_DIR`** — an explicit override. It exists so the game can be
   tested against a data directory without installing anything, and so a
   player can keep a modified copy of the tables.
2. **`./assets`** — running from the build tree, which is what you have
   done for twenty-three chapters. Developers should never have to install
   the thing to run it.
3. **`DATADIR`** — the installed location, baked in at compile time.

`getenv` returns `NULL` when a variable is unset, and — this is the part
people miss — an empty string when it is set but blank. `SAR_DATA_DIR=`
means "unset it", not "look in the root directory", so both are checked.

Note also that the check is `directory_exists`, not "can I open it":

```c
static int directory_exists(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        return 0;
    }

    /* S_ISDIR is a macro from <sys/stat.h>; a file named "assets" is not
       a data directory, and checking existence alone would accept one. */
    return S_ISDIR(st.st_mode);
}
```

### Where the save goes

Linux has a convention for this, the **XDG Base Directory specification**:
user data belongs in `$XDG_DATA_HOME`, and when that is unset, in
`$HOME/.local/share`.

```c
if (xdg != NULL && xdg[0] != '\0') {
    snprintf(dir, sizeof dir, "%s/some-assembly-required", xdg);
} else if (home != NULL && home[0] != '\0') {
    /* ... $HOME/.local/share/some-assembly-required ... */
}
```

Following it means the save turns up where a Linux user expects, and gets
backed up by whatever backs up their home directory. Inventing your own
location — `~/.somegame`, or worse, next to the binary — makes your program
that program.

Note `paths_save` *creates* the directory rather than assuming it, and
`ensure_directory` treats "already there" as success:

```c
static int ensure_directory(const char *path)
{
    if (directory_exists(path)) {
        return 1;
    }

    /* 0755: the owner may write, everyone may look. */
    return mkdir(path, 0755) == 0;
}
```

And when it cannot — no `HOME`, read-only filesystem — it degrades to
`save.txt` in the current directory and returns 0, rather than refusing to
save. Chapter 20's rule was never trust a file on disk; this is its
companion: never let a missing directory be fatal when a worse-but-working
answer exists.

### The call sites

The world's map paths lose their prefix:

```c
.map_path = "maps/grubbin_vale.map",
```

...and gain a resolution step in `world_create`:

```c
char full[512];
paths_data(full, sizeof full, world->areas[i].map_path);

world->maps[i] = map_load(full);
```

`SAVE_PATH` disappears entirely, because there is no longer a single answer:

```c
/* No SAVE_PATH constant any more: where a save may be written
   depends on the user, not the build. Ask paths_save(). */
```

`map_load`, `config_load`, `party_load_levels` and `save_read` are all
untouched — they always took a `const char *path` and never cared where it
came from. That is the interface holding up under a change nobody
anticipated when it was written.

## Apply it: the Makefile

### PREFIX and DESTDIR

```make
PREFIX  ?= /usr/local
DESTDIR ?=

BINDIR  = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/some-assembly-required
```

`?=` assigns *only if not already set*, so both can be overridden from the
command line without editing anything.

Two variables, two different jobs, and they are constantly confused:

- **`PREFIX`** is where the software will *live*. It is compiled in.
- **`DESTDIR`** is where files are *written right now*. It is not compiled
  in — it exists so a package builder can install into a staging directory
  and tar it up, while the program still believes it lives in `/usr/local`.

```bash
make install DESTDIR=/tmp/stage PREFIX=/usr/local
```

...produces `/tmp/stage/usr/local/bin/game`, containing the path
`/usr/local/share/...`. That is correct, and it is the whole reason both
exist.

### One place to quote

```make
OPT ?= -g
SAN ?=

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic $(OPT) $(SAN) \
         -DDATADIR='"$(DATADIR)"'
LDLIBS = -lncursesw $(SAN)
```

The first version of this chapter had `release` and `sanitize` each pass a
whole replacement `CFLAGS`, re-quoting `DATADIR` on the way. It broke
immediately:

```
<command-line>: error: expected expression before ‘/’ token
   53 |         snprintf(data_dir, sizeof data_dir, "%s", DATADIR);
```

The quotes did not survive two levels of `make`. Isolating the *one* thing
each variant changes — `OPT`, `SAN` — means `DATADIR` is quoted exactly
once, and the targets become one line each:

```make
release:
	$(MAKE) clean
	$(MAKE) OPT='-O2 -DNDEBUG'

sanitize:
	$(MAKE) clean
	$(MAKE) SAN=-fsanitize=address,undefined
```

### install

```make
install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 assets/levels.txt $(DESTDIR)$(DATADIR)/levels.txt
	install -Dm644 assets/spells.txt $(DESTDIR)$(DATADIR)/spells.txt
	install -Dm644 assets/config.txt $(DESTDIR)$(DATADIR)/config.txt
	for m in assets/maps/*.map; do \
	    install -Dm644 "$$m" $(DESTDIR)$(DATADIR)/maps/$$(basename $$m); \
	done
```

`install` is a real program, not a synonym for `cp`. `-D` creates missing
parent directories; `-m` sets permissions explicitly rather than inheriting
whatever the umask happens to be — `755` for the executable, `644` for data
nobody should be running.

The `$$m` is not a typo. `make` eats one `$`, so the shell sees `$m`. Every
shell variable inside a recipe needs doubling, and forgetting is a rite of
passage.

`uninstall` removes what was installed and deliberately does **not** touch
the player's saves:

```make
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(DATADIR)
	@echo "Removed. Saves in ~/.local/share are left alone."
```

Uninstalling a program should not delete a player's progress. If they want
it gone they can delete it; if you delete it for them, they cannot get it
back.

### dist

```make
DISTNAME = some-assembly-required-1.0
dist: clean
	mkdir -p $(DISTNAME)
	cp -r *.c *.h Makefile valgrind.supp assets $(DISTNAME)/
	tar czf $(DISTNAME).tar.gz $(DISTNAME)
	rm -rf $(DISTNAME)
```

The tarball unpacks into its own directory. An archive that scatters forty
files into whatever directory the user happened to be in is called a
*tarbomb*, and people remember them.

`dist` depends on `clean` so that object files and binaries cannot end up in
a source archive.

## Compile and run

```bash
make clean
make
ldd ./game
```

Then a full staged install and a run from somewhere else entirely:

```bash
make install PREFIX=/tmp/sar-test
cd /
/tmp/sar-test/bin/game
```

```
****************************************
*                                      *
*       SOME ASSEMBLY REQUIRED         *
*       A Quest in Four Pieces         *
****************************************
What is your name, hero?
```

It starts, from `/`, with no `assets` directory within a mile of it. That is
the chapter.

Confirm the path really is compiled in:

```bash
strings /tmp/sar-test/bin/game | grep some-assembly-required
```

```
/tmp/sar-test/share/some-assembly-required
```

The release build, and what it costs:

```bash
make release
```

|  | bytes |
|---|---|
| `make` (with `-g`) | 160,928 |
| `make release` (`-O2 -DNDEBUG`, no `-g`) | 73,696 |

Less than half, and the difference is almost entirely debug information —
useful to you, useless to a player.

A source release, checked by actually building it:

```bash
make dist
tar xzf some-assembly-required-1.0.tar.gz -C /tmp
cd /tmp/some-assembly-required-1.0 && make
```

Zero warnings, and a `game` binary. Then clean up:

```bash
make uninstall PREFIX=/tmp/sar-test
```

And the whole suite, one last time:

```
303274 checks, 0 failed
==299422== All heap blocks were freed -- no leaks are possible
==299422== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## What just happened

**A relative path is a hidden dependency on the working directory.** It
worked for twenty-three chapters because the working directory was always
the same one. The bug was there the whole time; only installing revealed it.

**Read-only data and writable data are different problems.** Shipped data is
found; user data is created. Putting them in the same place works only
while you are the only user, on a machine where you own everything.

**Compile time is a legitimate place to configure things.** Not everything
needs a config file. `DATADIR` cannot come from one — you would have to find
the config file first, which is the same problem again.

**Interfaces you got right pay you back years later.** `map_load(const char
*path)` was written in Chapter 11 with no thought of installation. Because
it took a path rather than reaching for a global, this chapter changed
*where paths come from* without touching it at all.

## Common errors

**Losing the quotes around a `-D` string:**

```make
CFLAGS = -DDATADIR="$(DATADIR)"
```

```
<command-line>: error: expected expression before ‘/’ token
```

The shell strips the double quotes, so the macro expands to bare tokens.
Use `-DDATADIR='"$(DATADIR)"'`.

**Forgetting to double `$` in a recipe:**

```make
	for m in assets/maps/*.map; do install -Dm644 "$m" ...; done
```

`make` expands `$m` as one of *its* variables — almost certainly empty —
and the shell receives nothing. Write `$$m`.

**Installing before compiling with the new `DATADIR`:**

```bash
make
make install PREFIX=/opt/games      # binary still says /usr/local
```

`DATADIR` is compiled in, so changing `PREFIX` at install time changes
where files are *put* but not where the program *looks*. Pass the same
`PREFIX` to both, or `make clean` between them.

**A missing library at run time:**

```
./game: error while loading shared libraries: libncursesw.so.6:
cannot open shared object file: No such file or directory
```

Not your program — the dynamic loader, before `main`. The machine needs the
library installed, or the binary needs to be linked statically.

**Saving next to the binary:**

```c
save_write("save.txt", ...);       /* relative to the working directory */
```

Works in the build tree; in an installed program it either writes to a
random directory or fails on a read-only one. Ask `paths_save`.

## Exercises

> **Practice drills:** `code/ch24/practice/` before exercise 2.

1. *Practice.* Path join, `--where` argv, DESTDIR vs PREFIX story.
2. *Durable — `--where`.* Print data dir + save path, then exit — keep the
   flag.
3. *Reading.* `ldd` on `./game` vs `run_tests`; `DESTDIR` install layout.
4. *Open-ended:* README install blurb for ncurses — static vs dynamic.

<details>
<summary>Solutions</summary>

1. Practice solutions.
2. Parse `argv` before `game_create`.
3. Tests omit ncurses; DESTDIR is write staging only.
4. Audience-dependent; document runtime deps either way.

</details>

## You are done

That is the course.

You started with a `printf` that put a title on a screen, and you have
finished with a program that has an overworld, towns, dungeons, NPCs,
turn-based combat with three boss fights, levelling, inventory, magic,
equipment, shops, save and load, an ending, a test suite of three hundred
thousand assertions, a fuzzer, a clean bill of health from two different
memory tools, and an installer.

Along the way you learned C — not a tour of it, the real thing. Pointers and
what they actually are. The heap, and who owns what on it. Why strings end
in a zero byte and what that costs. Undefined behaviour, named rather than
avoided. Function pointers, dispatch tables, variadic functions, the
compilation model, the linker, `gdb`, sanitizers, Valgrind.

The appendices are worth a look now rather than earlier:
[b-c-pitfalls](../appendices/b-c-pitfalls.md) collects the mistakes this
course walked you into on purpose;
[c-gdb-and-sanitizers](../appendices/c-gdb-and-sanitizers.md) is a working
reference for the tools;
[d-glossary](../appendices/d-glossary.md) defines every term the course
introduced; and
[e-stretch-goals](../appendices/e-stretch-goals.md) has four projects
sketched out — a bestiary, New Game+, a boat, and an SDL2 port — for
carrying on alone, which is the point.

The game is yours. The bad jokes about civil service were always yours.
Go and add something the tutorial never mentioned.
