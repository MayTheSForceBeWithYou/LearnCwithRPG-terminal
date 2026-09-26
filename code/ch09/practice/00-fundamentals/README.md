# 00 — Camera math fundamentals

Micro-drills before Monk's scenario folders (`01-page-snap`, etc.).
Each file is one program. No ncurses. No link against the game.

```bash
cd code/ch09/practice/00-fundamentals
make && ./01_clamp && ./02_center && ./03_bounds \
  && ./04_world_to_screen && ./05_visibility
make clean
```

| File | Skill |
|------|--------|
| `01_clamp.c` | `clamp` edge cases |
| `02_center.c` | center + clamp formula |
| `03_bounds.c` | `MAP - VIEW` upper bound; VIEW > MAP |
| `04_world_to_screen.c` | conversion + round-trip |
| `05_visibility.c` | half-open on-screen test |

Then continue with `../01-page-snap`, `../02-dead-zone`, `../03-clamp-and-center`.
