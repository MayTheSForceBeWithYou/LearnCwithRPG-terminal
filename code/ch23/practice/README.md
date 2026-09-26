# Chapter 23 practice — format attrs & phase tables

Do **not** delete the format attribute from production `log_linef`.

## Order

1. **01-format-contract/** — `LESSON.md` / `TASK.md`; `make check` then
   `make bad` (with vs without attribute on intentional mismatch).
2. Micros: `01_format_attr`, `02_phase_index`, `03_sim_potion_gate`.

```bash
cd 01-format-contract && make && make check && make bad && make clean
cd .. && make && make check && make clean
```
