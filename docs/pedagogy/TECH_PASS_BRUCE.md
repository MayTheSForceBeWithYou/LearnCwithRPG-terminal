# Tech pass — Bruce (final before PR)

**Date:** 2026-09-25 ~11:20 PM PT  
**Tree:** `/home/n8/dev/c/LearnCwithRPG-terminal`  
**Inputs:** `HANDOFF_FOR_BRUCE.md`, `FIXES_APPLIED_MUSTFIX_BATCH.md`, `STYLE_REVIEW_MUSTFIX_BATCH.md`, `INSTRUCTIONAL_AUDIT.md`

## Verdict

**Ready for branch / commit / PR: YES** (technical), with residual risks below (non-blocking).

## Verified

| Check | Result |
| ----- | ------ |
| Genre-tag (old J-prefixed acronym) | **0** hits (scrubbed pedagogy meta wording) |
| Chapter Exercises smash production | **Clean** — no unprotected instruct to delete `memset`, damage floor, format attr, sortedness, or corrupt shipped `overworld.map` |
| ch09 `camera_is_on_screen` | Present in `camera.h` / `camera.c` / `main.c`; exercises forbid replacing lasting `camera_center_on` |
| ch09 practice vs game path | Page-snap / dead-zone stay under `practice/`; game keeps clamped follow |
| Practice solutions compile+run | **83 OK / 0 FAIL** |
| Format contract `make bad` | without_exit=0, with_exit=1 on gcc `-Werror=format` (as handoff) |
| Assertions `make check` / `demo-assert` | Green default; demo build via `-DDEMO_BAD_FLOOR` only |
| Appendices B/C | Added **Related practice** cross-links to ch05/07/10/22 drills |
| Artifacts | ELF binaries / `.o` under `code/` cleaned after verification |

## Fixed this pass

1. **`code/ch21/practice/02-loader-invariants/solutions/main.c`** — parity with Ch11; SOLUTION.md points at it.
2. Genre-tag wording scrub in `docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md`.
3. Appendix B (`b-c-pitfalls.md`) + C (`c-gdb-and-sanitizers.md`) practice cross-links.
4. Full tree artifact cleanup after compile sweep.

## Intentional stub failures (students fill TODOs)

These **compile** but **fail checks** until TODOs are done (by design):

| Path | Notes |
| ---- | ----- |
| `code/ch15/practice/01_clamp_damage.c` | TODO stub |
| `code/ch15/practice/02_flee_chance.c` | TODO stub |
| `code/ch21/practice/01-spell-line/main.c` | TODO parse |
| `code/ch21/practice/02-loader-invariants/main.c` | TODO1–3 |

**Already-green student files (not TODO stubs):**

- `code/ch15/practice/03_monotonic.c`, `04_xorshift_seed.c` — complete teaching programs
- `code/ch22/practice/01-tool-coverage/main.c` — observational (unset `ready`)
- `code/ch22/practice/02-assertions-vs-tests/main.c` — floor on; bad floor only via `make demo-assert`
- `code/ch23/practice/01-format-contract/main.c` — good call only; mismatch in `bad_call.c` + `make bad`
- Many earlier-chapter micros ship complete solutions separately under `solutions/`

Other chapter practice stubs (ch02–14,16–20,23 `02_phase_index`, etc.) likewise expect FAIL until filled; solutions under `solutions/` (or `SOLUTION.md` / `solutions_main.c`) are the oracle.

## Residual risks (non-blocking)

1. **No full-game `make valgrind` / ASan** run in this pass (handoff optional). Recommend once on WSL before merge if CI lacks it.
2. **Monk concurrent trees** — if further edits land after this stamp, re-run genre + vandalism scan.
3. **`make check` naming** — scenario folders use `make check`; older micros use run-the-binary; both documented in READMEs.
4. **ch23 `02_phase_index.c` stub** may warn unused `lines` until TODO — acceptable per handoff; solutions clean.
5. Raw sweep log: `docs/pedagogy/_tech_pass_raw.txt` (can delete in PR hygiene if desired).

## Explicit non-goals

- No commit / branch / push / PR (parent owns git).
- No ch09 rewrite.
- No production `combat_math.c` / `magic.c` / `entity_create_player` vandalism for demos.
