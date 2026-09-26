# Solution — 01-format-contract

With the attribute, gcc errors on log_linef("value=%s", x).
Without it, the compile may succeed and runtime is undefined.

Do **not** delete the format attribute from production log_linef.
