# CLAUDE.md

Persistent instructions for this repository. Read at the start of every session.

---

## Prime Directive

**This repository contains a tutorial course, not a game.**

We are writing a course that teaches a human to build a JRPG in C. The learner writes
the game. You write the lessons that make that possible.

Yes, you will write code — every chapter has examples, and every chapter ships a
compiling snapshot under `code/`. That is required. What is forbidden is getting ahead
of the curriculum: implementing a subsystem and then back-filling lessons that explain
it. Build the course in chapter order. If you catch yourself about to "just implement
the battle system so we have something to explain," stop and reread `DESIGN.md` §0.

---

## Authoritative Files

| File | Role |
|---|---|
| `DESIGN.md` | The specification. Curriculum, chapter list, checkpoints, standards. Authoritative — where your instincts disagree with it, follow it. |
| `PROJECT_CHOICES.md` | The learner's decisions, recorded at each checkpoint. Authoritative for all branching content. |
| `CLAUDE.md` | This file. Behavioural rules that apply every turn. |

`DESIGN.md` is long. Read it fully at the start of a work session, not just when stuck.

---

## Session Start Protocol

At the beginning of every session, before doing anything else:

1. Read `DESIGN.md` and `PROJECT_CHOICES.md`.
2. Determine current state: find the highest-numbered file in `chapters/` and the
   highest-numbered directory in `code/`. They should match. If they don't, that
   mismatch is the first thing to fix.
3. Check `git log --oneline | head -20` for recent work.
4. Report in two or three sentences: which chapter is complete, what the next chapter
   is, and whether a checkpoint stands between here and there.
5. Then proceed — or wait, if a checkpoint is next.

Never assume you remember the state from a previous session. Verify it from disk.

---

## Non-Negotiable Rules

1. **Stop and wait at every checkpoint.** Present the questions, then end the turn.
   Do not ask a question and answer it yourself. Do not proceed on assumed answers.
   The learner explicitly asked to be consulted; this is a feature, not friction.

2. **One chapter at a time, verified before moving on.** Write the chapter, build its
   snapshot, compile it, run it, fix what's broken. Only then start the next chapter.
   Never batch chapters ahead of verification.

3. **No concept before its spotlight.** Before writing any code into a chapter, ask:
   has every C feature in this code already been taught? Pointers before chapter 7,
   `malloc` before chapter 10, function pointers before chapter 14 — all violations.
   If the game needs a concept early, either restructure the code to avoid it or move
   the concept's chapter earlier and update `DESIGN.md`.

4. **Every snapshot compiles clean and runs.** Zero warnings under the standard flags.
   A snapshot that doesn't build is worse than no snapshot — it destroys the learner's
   trust in their safety net.

5. **Snapshots are cumulative and minimal.** `code/chNN/` is `code/ch(NN-1)/` plus
   exactly that chapter's changes. `diff -r` between consecutive snapshots should show
   only what the chapter taught. No drive-by refactors, no tidying up unrelated files.

6. **Record every choice.** After a checkpoint, append the question, options, chosen
   answer, and date to `PROJECT_CHOICES.md`. Later chapters must reference it explicitly
   in prose ("You chose an MP pool at Checkpoint D, so...").

7. **Commit after each chapter.** `git add -A && git commit -m "ch07: pointers and
   collision"`. History is how the learner and future sessions see what changed.

---

## Failure Modes — Self-Check Before Every Chapter

These are the ways this project goes wrong. Check against them explicitly.

- **Building the game.** Am I writing lessons, or am I writing a game with commentary?
- **Front-running the curriculum.** Does this code use anything not yet taught?
- **The 300-line dump.** Am I showing a diff and a changed function, or pasting a file?
- **Silent perfection.** Am I presenting a polished subsystem that should have arrived
  naive first and improved over two or three chapters?
- **Answering my own question.** Did I just decide something the learner should decide?
- **Skipping the C half.** Does this chapter have a real concept spotlight with a
  standalone example, or did I go straight to game code?
- **Untested prose.** Did I actually run the commands I'm telling the learner to run?
- **Drift.** Does this chapter contradict a choice recorded in `PROJECT_CHOICES.md`?

---

## Build & Verify

Standard flags for every snapshot:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g
```

Verification loop after writing `code/chNN/`:

```bash
cd code/chNN
make clean && make          # must produce zero warnings
./game                      # must run and do what the chapter claims
cd ../.. && diff -r code/ch$((NN-1)) code/chNN   # review the delta
```

From chapter 10 onward, also verify under sanitizers:

```bash
make clean && make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined"
./game
```

Warnings are errors in spirit. Do not leave them for the learner to trip over.

---

## Environment

Target is **WSL2 running Arch Linux**. Development and play both happen there.

- Verify package names against current Arch repos before writing install commands.
  Do not trust memory for package naming.
- The project lives in the Linux filesystem (`~/...`), never under `/mnt/c/`. If any
  instruction implies otherwise, it is wrong.
- Renderer choice is recorded in `PROJECT_CHOICES.md` (Checkpoint A). Package lists,
  the `Makefile`, and all rendering prose follow from it.
- WSL-specific gotchas belong in `appendices/a-wsl-troubleshooting.md`, not scattered
  through chapters. Cross-reference from chapters instead of repeating.

---

## Code Standards

Applies to every line of C written anywhere in this repo.

- C17. `snake_case` functions and variables, `PascalCase` for `typedef`'d types,
  `SCREAMING_CASE` for macros and enum constants.
- Module-prefixed public functions: `map_load`, `battle_begin`, `party_heal`.
- Paired lifetimes: `foo_create` / `foo_destroy`. Every allocation has a documented owner.
- Check the return of `malloc`, `fopen`, and every allocating call. Every time, from the
  first allocation onward. Model the discipline; never write "error handling omitted for
  brevity."
- `snprintf` over `sprintf`. `fgets` over anything else for line input.
- 4-space indent, no tabs. ~90-column soft limit so code blocks don't wrap in markdown.
- Include guards on every header.
- Functions under ~50 lines. When one grows past that, split it and explain why in prose.
- Total project stays under ~5,000 lines. If a design pushes past it, simplify the
  design, not the explanation.

---

## Chapter File Conventions

`chapters/chNN-slug.md`, following the nine-part structure in `DESIGN.md` §5.3:
where we are → the problem → C concept spotlight → apply it → compile and run →
what just happened → common errors → exercises → next up.

- Fenced code blocks always tagged (` ```c `, ` ```bash `, ` ```make `).
- Say exactly where code goes: *"in `entity.h`, below the `Vec2` definition"* — never
  "add this to the file."
- "Common errors" contains real, verbatim compiler or runtime output. Generate it by
  actually making the mistake and copying what the toolchain says.
- Exercise solutions go in collapsed `<details>` blocks.
- ASCII memory diagrams for anything involving pointers, the stack, or the heap.

---

## Tone

Direct, warm, occasionally funny. Confidence-building without cheerleading.

"This next part is genuinely tricky and most people get it wrong the first time" — good.
"This is easy!" — never. It isn't, and saying so only makes a stuck learner feel stupid.

Define jargon inline, in one sentence, on first use. Translation unit, undefined
behaviour, dangling pointer: all need a gloss the first time they appear.

---

## When The Learner Wants To Deviate

They may want to skip ahead, revisit a checkpoint, or change an earlier decision.
That's fine — it's their course. But:

1. Update `PROJECT_CHOICES.md` with the change and the date.
2. Identify which already-written chapters are now inconsistent, and say so plainly.
3. Offer to fix them, or to flag them with a note, and let the learner choose.

Never silently leave the course self-contradictory. A tutorial that disagrees with
itself is worse than one that admits a loose end.
