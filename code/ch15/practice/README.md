# Chapter 15 practice — pure combat math & RNG

Standalone. No ncurses. Re-implement tiny pure helpers; property tests here.
(Bruce micro-drills — complete these before the chapter's durable harness /
flee wiring exercises.)

Chapter Exercises point here first: damage floor, flee chance, monotonicity,
seeded RNG. Do **not** delete the production damage floor to experiment —
that belongs in later Ch22 drill `02-assertions-vs-tests` with
`-DDEMO_BAD_FLOOR`, not in the lasting combat maths.

```bash
cd code/ch15/practice
make && make check
make clean
```

Solutions under `solutions/` — try the stubs first.
