# Solution

```c
static int parse_spell_line(const char *line, Spell *out)
{
    char name[32];
    if (sscanf(line, "%d %d %d %15s %31[^\n]",
               &out->level, &out->mp, &out->mag, out->effect, name) < 5)
        return 0;
    /* trim trailing newline already excluded by scanset */
    snprintf(out->name, sizeof out->name, "%s", name);
    return 1;
}
```
