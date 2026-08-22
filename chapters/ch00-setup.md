# Chapter 0: Setting Up WSL Arch

## Where we are

Nowhere, yet — and that's fine. You have WSL2 with Arch Linux installed, and
that's the only prerequisite for this course. By the end of this chapter
you'll have a working C toolchain and you'll have compiled and run your
first program. Nothing about the game exists yet; this chapter is entirely
about making sure the ground under you is solid before you build on it.

## The problem

A compiler doesn't ship with Arch Linux by default, and neither do the other
tools you'll lean on constantly through this course: a debugger, a memory
checker, and the display library the game will draw with. Installing the
wrong thing, or installing it in the wrong place, causes confusing failures
weeks from now that look like they're your fault when they're actually an
environment problem. Get this right once, here, and you won't think about it
again until Chapter 4 (when we explain what these tools actually do).

## Toolchain concept spotlight: what you're about to install, and why

You don't need to understand these deeply yet — later chapters do that job.
You just need to know what each one is *for*, so the install commands below
aren't a wall of unexplained nouns.

- **`gcc`** — the GNU Compiler Collection. Turns the C you write into a
  program the CPU can run. This is the one tool you cannot substitute; every
  other tool here supports it.
- **`make`** — a build automation tool. You won't use it until Chapter 4,
  but it comes bundled in the `base-devel` group, so we grab it now.
- **`gdb`** — the GNU Debugger. Lets you pause a running program and inspect
  its state. You'll meet it properly in Chapter 10, right after you meet
  your first heap bug.
- **`valgrind`** — a tool that runs your program inside a simulator and
  reports memory mistakes (leaks, use of freed memory) that would otherwise
  fail silently. Also Chapter 10 onward.
- **`git`** — version control. Not a C tool at all, but you should be
  committing your work as you go, and `base-devel` doesn't include it.
- **`pkgconf`** — helps the build system find libraries (like `ncurses`)
  that are installed on your system. You won't touch it directly for a
  while, but `make` will need it once the game links against `ncurses`.
- **`ncurses`** — the terminal display library this course uses to draw the
  game. **This is a project-specific choice, not something every C project
  needs** — you picked it at Checkpoint A over SDL2's windowed graphics,
  because it keeps all your attention on C itself with nothing extra to
  configure. If you ever migrate to SDL2 (Chapter 8 builds the abstraction
  that makes this possible later), you'd install `sdl2`, `sdl2_image`,
  `sdl2_ttf`, and `sdl2_mixer` instead.

*Translation unit, undefined behaviour,* and friends are coming — but not
today. Today is just tools.

## Apply it

### 1. Confirm you're in the Linux filesystem

Before installing anything, check where you are:

```bash
pwd
```

If the output starts with `/mnt/c/`, stop and move. Windows drives mounted
into WSL are dramatically slower for the kind of small, frequent file access
a compiler does, and they mangle Unix file permissions in ways that produce
baffling `make` errors much later. Your project must live under your Linux
home directory instead:

```bash
mkdir -p ~/projects
cd ~/projects
```

Do all your work for this course from here on, inside the Linux filesystem.
This one decision prevents a whole category of problems you'd otherwise
spend hours debugging later, convinced the bug is in your code.

### 2. Update the system and install the toolchain

```bash
sudo pacman -Syu
sudo pacman -S --needed base-devel gdb valgrind git pkgconf
sudo pacman -S --needed ncurses
```

`base-devel` is a package *group* — it pulls in `gcc`, `make`, and a handful
of other build essentials in one shot. `--needed` tells `pacman` to skip
anything already installed rather than reinstalling it, which makes this
command safe to re-run if you're not sure what you already have.

If `pacman -Syu` fails with keyring or clock-related errors on a freshly
imported Arch image, see
[Appendix A](../appendices/a-wsl-troubleshooting.md#pacman-key---init--keyring-failures).

### 3. Check your terminal

The ncurses track draws with Unicode box-drawing characters and color, and
not every terminal renders those correctly.

```bash
echo $TERM
```

You want something like `xterm-256color`, not `dumb` or empty. If you're
using **Windows Terminal** (recommended — search for it in the Microsoft
Store if you don't have it), this is rarely a problem. If characters look
wrong later when we start drawing maps, see
[Appendix A](../appendices/a-wsl-troubleshooting.md#terminal-rendering-looks-wrong-ncurses-track).

### 4. Pick an editor

Any of these work fine; the tutorial doesn't assume one:

- **VS Code** on Windows, with the **WSL extension** — opens files inside
  your Arch filesystem directly, with full IntelliSense once you have a
  `compile_commands.json` (not needed yet).
- **Neovim**, installed inside WSL (`sudo pacman -S neovim`) and run from
  the same terminal you're compiling in.

Use whichever you already know. Don't let editor choice become a detour.

### 5. Sanity-check the toolchain

Create a scratch file — not part of the game, just proof everything works:

```bash
mkdir -p ~/projects/jrpg-sanity-check
cd ~/projects/jrpg-sanity-check
```

Create `hello.c`:

```c
#include <stdio.h>

int main(void)
{
    printf("Hello, adventurer!\n");
    return 0;
}
```

We're not explaining this program yet — `#include`, `main`, and `printf`
all get their proper introduction in Chapter 1. This is purely a toolchain
check.

## Compile and run

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o hello hello.c
./hello
```

Expected output:

```
Hello, adventurer!
```

If you see that line, your entire toolchain — compiler, linker, and the
flags this whole course uses — works. That's the goal for today.

## What just happened

The `gcc` command you just ran did several things behind the scenes to turn
`hello.c` into the `hello` executable that `./hello` then ran. We're
deliberately not opening that box yet — Chapter 1 walks through the
preprocess → compile → assemble → link pipeline in detail, using this exact
program as the example. For now, treat `gcc ... -o hello hello.c` as a
single unit: "compile this file into this program." (Chapter 4 revisits
linking again once your project spans *multiple* files, where it gets more
interesting.)

## Common errors

**Installing without `sudo`:**

```
$ pacman -S base-devel
error: you cannot perform this operation unless you are root.
```

`pacman` manages system-wide packages, so it needs root privileges. Prefix
the command with `sudo` and enter your password when prompted.

**Typo in a package name:**

```
$ pacman -Si nonexistent-package-xyz
error: package 'nonexistent-package-xyz' was not found
```

Arch package names are usually the project's name in lowercase, but not
always. If a name from this chapter doesn't resolve, search for it with
`pacman -Ss <keyword>` before assuming the instructions are wrong.

**Missing semicolon:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o hello hello.c
hello.c: In function ‘main’:
hello.c:5:35: error: expected ‘;’ before ‘return’
    5 |     printf("Hello, adventurer!\n")
      |                                   ^
      |                                   ;
    6 |     return 0;
      |     ~~~~~~
```

C statements end with a semicolon; the compiler noticed the next line
(`return 0;`) starting where it expected one. The caret points at *where the
compiler noticed the problem*, which is often one line after where you
actually made the mistake — get used to checking the line above a reported
error, not just the line itself.

**Running a program that wasn't compiled (or was compiled somewhere else):**

```
$ ./hello
bash: ./hello: No such file or directory
```

Either the `gcc` command above didn't succeed (scroll up for the real
error) or you're in a different directory than the one you compiled in.
Check both with `pwd` and `ls`.

## Exercises

1. Run `gcc --version`. What version did Arch give you? (Anything recent
   enough to support `-std=c17` is fine — Arch tracks upstream closely, so
   this is essentially never a problem, but it's worth knowing how to check.)
2. Deliberately remove the closing `"` from the string in `hello.c` and
   recompile. Read the error. Is it the same *kind* of error as the missing
   semicolon above, or different? What does the compiler think went wrong?
3. Run `man gcc` and find the description of the `-Wall` flag in your own
   words. (You don't need to understand every warning it enables yet —
   Chapter 1 covers why this flag matters.)
4. *Open-ended:* Sketch, on paper or in a text file, what you think your
   game's title screen might eventually say — kingdom name, hero name,
   anything. Nothing here is binding; you're not committing to it until
   Checkpoint B. This is just to have something in mind as you build.

<details>
<summary>Solutions</summary>

1. There's no single right answer — report whatever `gcc --version` prints.
   As of this writing, Arch's `gcc` package is on the 16.x series, which
   supports `-std=c17` without issue (GCC has supported C17 since version
   8). If yours is dramatically older, `sudo pacman -Syu` and reinstall.

2. Different — and arguably more confusing. An unterminated string makes
   the compiler treat everything after the missing `"` as part of the
   string literal, so the error often points much further down the file
   (sometimes at the closing brace `}` of `main`, reported as a "missing
   terminating character" or similar) rather than exactly at your typo.
   This is a preview of a general truth about compiler errors: the reported
   location is where the compiler *noticed* something was wrong, not
   necessarily where you *caused* it.

3. `-Wall` enables "all the warnings the GCC developers consider generally
   useful" — despite the name, it is not actually *all* warnings (that's
   part of why this course also adds `-Wextra` and `-Wpedantic`). It's a
   good default that catches a wide range of common mistakes: unused
   variables, comparisons that are always true/false, and more.

4. No solution — this is yours. Keep it somewhere; Checkpoint B will ask
   about it directly.

</details>

## Next up

Chapter 1 puts that four-line `hello.c` under a microscope: what `main` is,
what `printf` actually does, and what the compiler is silently protecting
you from with `-Wall`. Your title screen starts there.
