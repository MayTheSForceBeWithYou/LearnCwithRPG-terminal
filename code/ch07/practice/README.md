# Chapter 7 practice — pointers

Standalone drills. No game link. Complete `01`–`05` before the chapter's
integration exercise. Solutions under `solutions/`.

```bash
cd code/ch07/practice
make
./01_swap && ./02_minmax && ./03_walk_array && ./04_const_contract && ./05_dangling_demo
make clean
```

| File | Skill |
|------|--------|
| `01_swap.c` | Classic `swap(int*, int*)` |
| `02_minmax.c` | Two out-params |
| `03_walk_array.c` | Pointer vs index walk |
| `04_const_contract.c` | Why `const T *` rejects writes |
| `05_dangling_demo.c` | Returning `&local` is wrong (see comments) |
