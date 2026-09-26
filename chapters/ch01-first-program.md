# Chapter 1: Your First Program

## Where we are

Chapter 0 got your toolchain working and proved it with a four-line
`hello.c` that printed a greeting. You ran one `gcc` command and trusted
that it did the right thing. This chapter opens that box: what `main` and
`printf` actually are, what `gcc` does in between "source file" and
"running program," and why the `-Wall -Wextra -Wpedantic` flags you've been
typing matter. By the end, your project has its first real file — `main.c`
— and it prints an actual (if placeholder) title screen.

## The problem

You can't build a RPG without understanding the two lines every C program
starts from: the function that runs first, and the call that put text on
your screen in Chapter 0. Right now those are magic incantations you copied.
Magic incantations break in ways you can't fix. Let's fix that before you
write another line of game code.

## C concept spotlight: from source file to running program

### `main`, precisely

```c
int main(void)
{
    return 0;
}
```

`main` is not a function you call — it's the function the operating system
calls, once, when your program starts. Three things about the line above,
each deliberate:

- **`int`** is `main`'s return type. That integer becomes your program's
  *exit status* — a number the shell can check to ask "did that succeed?"
  `0` conventionally means success; anything else means failure, and the
  specific nonzero value is yours to define. Try it: after compiling any
  program in this course, run `echo $?` immediately afterward — that's the
  shell showing you the exit status of the last command.
- **`(void)`** means "this function takes no arguments." In C, empty
  parentheses (`main()`) mean something different and looser (unspecified
  arguments) — always write `(void)` when you mean "none." You'll see
  `main` take real arguments (`int argc, char *argv[]`) far later, if at
  all, in this course.
- **`return 0;`** hands that exit status back to the operating system. In
  `main` specifically — and only in `main` — falling off the end of the
  function without a `return` is allowed and implicitly means
  `return 0;`. Every other function in this course requires an explicit
  `return`; don't rely on this exception elsewhere.

### `printf`, precisely

```c
printf("Hello, adventurer!\n");
```

`printf` isn't a language keyword — it's an ordinary function, declared in
the header `<stdio.h>` and implemented in the C standard library. That's
why `hello.c` had `#include <stdio.h>` at the top: without it, the compiler
has no idea `printf` exists. Try deleting that `#include` line and
recompiling — the "Common errors" section below shows you exactly what
happens, because we actually did it.

`\n` inside the string is an **escape sequence**: two characters in your
source file (`\` and `n`) that the compiler turns into one character in
memory — a newline. You can't type a literal newline inside a normal string
literal, so C gives you this notation instead.

### The pipeline: what `gcc` actually does

That one `gcc` command from Chapter 0 is really four stages glued together.
You can run each one by hand and watch the file change shape. Try this with
a fresh three-line addition to `hello.c` — add a macro:

```c
#include <stdio.h>

#define GREETING "Hello, pipeline!"

int main(void)
{
    printf("%s\n", GREETING);
    return 0;
}
```

Save this as a scratch file, `greet.c`, anywhere outside your project (this
isn't part of the game). Then:

**Stage 1 — Preprocessing** (`-E`): handles anything starting with `#`
— `#include`, `#define` — before real compilation begins.

```bash
gcc -E -std=c17 greet.c -o greet.i
wc -l greet.i
```

```
566 greet.i
```

Open `greet.i` and look at the bottom. Somewhere around line 560 you'll
find your `main` function again — but changed:

```c
int main(void)
{
    printf("%s\n", "Hello, pipeline!");
    return 0;
}
```

`GREETING` is gone, replaced by the literal string it stood for — the
preprocessor did a pure text substitution, exactly like find-and-replace.
And the other 560-odd lines? That's `<stdio.h>` and everything *it*
includes, pasted in wholesale. `#include` is textual, not magical: it
splices the named file's contents directly into yours before anything else
happens. This is also your first hint of why compiling C can feel slow —
even a four-line program secretly compiles hundreds of lines.

**Stage 2 — Compilation** (`-S`): translates the preprocessed C into
assembly — a human-readable (if terse) text form of CPU instructions.

```bash
gcc -S -std=c17 -Wall -Wextra -Wpedantic greet.c -o greet.s
cat greet.s
```

```
	.file	"greet.c"
	.text
	.section	.rodata
.LC0:
	.string	"Hello, pipeline!"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 16.2.1"
	.section	.note.GNU-stack,"",@progbits
```

You don't need to read assembly to get the point: your string literal
became data (`.LC0`), and your `printf` call became a handful of CPU
instructions plus a `call`. One curiosity worth naming: it says
`call puts@PLT`, not `call printf`. GCC noticed your format string was just
`"%s\n"` with a single string argument — nothing `printf`'s extra power was
needed for — and silently substituted the simpler `puts` function. This
kind of compiler cleverness is normal and safe; don't worry about it, and
don't expect to predict it.

**Stage 3 — Assembly** (`-c`): turns that assembly text into actual machine
code, stored in an **object file** (`.o`) — a binary format, not something
you read directly.

```bash
gcc -c -std=c17 -Wall -Wextra -Wpedantic greet.c -o greet.o
file greet.o
```

```
greet.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
```

"Relocatable" means this object file isn't a runnable program yet — it's a
chunk of machine code with a gap where `puts` should be called, waiting to
be told where `puts` actually lives.

**Stage 4 — Linking**: fills in that gap. The C standard library (where
`puts`/`printf` actually live) gets combined with your object file into one
executable.

```bash
gcc greet.o -o greet
./greet
```

```
Hello, pipeline!
```

When your project is a single file, `gcc source.c -o program` runs all four
stages back to back and throws away the intermediate `.i`/`.s`/`.o` files.
That's what you did in Chapter 0. Once your project spans *multiple* files
(Chapter 4), you'll compile each one separately into its own `.o` and link
them together explicitly — and that's when "linking" gets interesting,
because it's also when it can fail with errors like `undefined reference
to 'foo'`. File that phrase away; you'll meet it for real in Chapter 4.

### What `-Wall -Wextra -Wpedantic` are actually doing

These flags don't change what your program *does* — they change what the
compiler is willing to warn you about while producing it. `-Wall` turns on
a broad set of warnings for common mistakes (despite the name, it is not
literally "all" warnings — that's a historical naming remnant, not a
modern accurate description). `-Wextra` adds more, pickier ones. `-Wpedantic` warns about
places your code relies on a compiler-specific extension instead of
standard C. Together, they turn silent bugs into loud ones, on your
terminal, before you've even run the program. See "Common errors" below for
one caught red-handed.

## Apply it

Time to start the actual project. Everything in this course from here on
lives in a real directory — not the scratch files from Chapter 0.

```bash
mkdir -p ~/projects/rpg
cd ~/projects/rpg
```

Create `main.c`:

```c
#include <stdio.h>

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");
    printf("(Placeholder title. You'll name this kingdom\n");
    printf("for real at Checkpoint B.)\n");

    return 0;
}
```

The `UNTITLED RPG` name is a deliberate placeholder — you haven't named
your kingdom or your hero yet. That happens at Checkpoint B, before Chapter
13, once the game actually has a world worth naming. For now, this is just
proof that `printf` can draw something that *feels* like a title screen.

## Compile and run

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
./game
```

Expected output:

```
****************************************
*                                      *
*           UNTITLED RPG               *
*                                      *
****************************************

(Placeholder title. You'll name this kingdom
for real at Checkpoint B.)
```

Zero warnings from `gcc`. If you see any, read them — don't ignore them,
even (especially) if the program still ran. That's next.

## What just happened

Your `main.c` went through the exact four-stage pipeline from the spotlight
section above: `#include <stdio.h>` got spliced in during preprocessing
(hundreds of lines, same as `greet.c`), each `printf` call became a `call`
instruction during compilation, assembly turned that into machine code in a
throwaway object file, and linking stitched in the standard library's
implementation of `printf`/`puts` to produce `game`. You didn't see any of
the intermediate files this time because plain `gcc main.c -o game` deletes
them automatically once linking succeeds — they only stuck around earlier
because you asked for them explicitly with `-E`, `-S`, and `-c`.

## Common errors

**Forgetting `#include <stdio.h>`:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
main.c: In function ‘main’:
main.c:3:5: error: implicit declaration of function ‘printf’ [-Wimplicit-function-declaration]
    3 |     printf("Hello, adventurer!\n");
      |     ^~~~~~
main.c:1:1: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
  +++ |+#include <stdio.h>
    1 | int main(void)
main.c:3:5: warning: incompatible implicit declaration of built-in function ‘printf’ [-Wbuiltin-declaration-mismatch]
    3 |     printf("Hello, adventurer!\n");
      |     ^~~~~~
```

Modern GCC treats calling an undeclared function as an outright **error**,
not just a warning — older compilers (and older tutorials) let this slide
with just a warning, which is worse: your program would still compile and
often still work, right up until it didn't, on a different compiler or a
different function. The compiler is telling you exactly what to do: add the
`#include`.

**A variable you declared but never used:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.c
main.c: In function ‘main’:
main.c:5:9: warning: unused variable ‘gold’ [-Wunused-variable]
    5 |     int gold = 100;
      |         ^~~~
```

Unlike the missing-`#include` error above, this is only a **warning** — the
compiler still produced `game`, and it would still run fine. That's exactly
why "warnings are errors in spirit" (a maxim we will ingrain in you during
this course): nothing forces you to fix this, but an unused variable is
almost always a sign you forgot to do something with it. Get in the habit of
reading every warning `gcc` prints, every time, even when the program "works."

## Exercises

1. Add `echo $?` right after running `./game`. What number do you see? Now
   change `return 0;` to `return 1;`, recompile, rerun both commands. What
   changed, and why didn't the printed title screen change too?
2. Deliberately delete the `#include <stdio.h>` line from your `main.c`,
   recompile, and read the error. Does it match the "Common errors" example
   above? Put the include back before moving on.
3. Run `gcc -E -std=c17 main.c -o main.i` on your actual `main.c` and check
   its line count with `wc -l`. Is it close to the 566 lines from `greet.i`
   in the spotlight section? Why would it be almost exactly the same
   despite your program being different?
4. *Open-ended:* Your title screen currently prints one static block of
   text. Sketch (in comments, or on paper — don't build it yet) what a
   *fancier* text title screen might include: ASCII art, a version number,
   a "controls" line. You'll get real tools for this kind of layout
   starting in Chapter 5 (arrays) and Chapter 12 (strings) — for now, just
   dream a little.

<details>
<summary>Solutions</summary>

1. `echo $?` after `return 0;` prints `0`; after changing to `return 1;`
   and recompiling, it prints `1`. The printed title screen is identical in
   both cases because the exit status is a completely separate channel from
   anything printed to the screen — `printf` writes to *standard output*,
   while the return value only becomes visible to whatever process launched
   yours (here, your shell) *after* the program has already finished
   running and closed.

2. Yes — you should see the same `implicit declaration of function
   'printf'` error, because the compiler genuinely has no declaration for
   `printf` without the header that provides it. The exact line numbers
   will differ slightly since your file's shape isn't identical to the
   error example, but the error category is the same.

3. It should be very close — likely within a handful of lines. Almost all
   of that preprocessed size comes from `<stdio.h>` itself (and whatever
   *it* transitively includes), which doesn't change based on what your
   `main` function does. Your actual code is a tiny fraction of what the
   compiler ends up processing — most of it is standard library
   declarations you never wrote and rarely read.

4. No fixed solution — this is a sketch for your own reference. Revisit it
   once Chapter 12 (Strings) covers word wrap and text boxes; you'll have
   the tools by then to build something closer to what you imagined here.

</details>

## Next up

A static title screen is nice, but a game needs to know things — a hero's
name, their starting stats. Chapter 2 covers reading input from the
keyboard and the handful of basic types (`int`, `char`, `float`) you'll
store it in.
