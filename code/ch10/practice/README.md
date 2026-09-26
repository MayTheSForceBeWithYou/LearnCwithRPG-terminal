# Chapter 10 practice — heap ownership

Standalone alloc/free drills. No ncurses. No game `Map`. Sanitize locally
when a drill says so (`-fsanitize=address,undefined`).

```bash
cd code/ch10/practice
make
./01_malloc_free && ./02_null_check && ./03_grow_buffer && ./04_owner_pair
make clean
```

Optional: rebuild `02` / a deliberate UAF toy with ASan after you read the
chapter's Bug sections — do that in a throwaway file, not by breaking
`code/ch10/map.c` permanently.
