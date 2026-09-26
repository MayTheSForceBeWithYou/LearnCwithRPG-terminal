# Solution — 02-loader-invariants

Loop levels for is_sorted_by_level. Strcmp effect against KNOWN_EFFECTS. validate_table: if unsorted or unknown effect, snprintf reason and return 0.

On unsorted.txt, prefix known_count at hero level 2 disagrees with a full scan — the menu would show the wrong spell. That is why the loader rejects unsorted input at the boundary.

Do **not** remove the sortedness check from code/ch21/magic.c.

Reference implementation: `solutions/main.c` (compile from this directory so `fixtures/` resolves).
