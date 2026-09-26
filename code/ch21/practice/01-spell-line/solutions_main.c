#include <stdio.h>
#include <string.h>
typedef struct { int level, mp, mag; char effect[16]; char name[32]; } Spell;
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
static int parse_spell_line(const char *line, Spell *out)
{
    char name[32];
    if (sscanf(line, "%d %d %d %15s %31[^\n]",
               &out->level, &out->mp, &out->mag, out->effect, name) < 5)
        return 0;
    snprintf(out->name, sizeof out->name, "%s", name);
    return 1;
}
int main(void)
{
    Spell s;
    check(parse_spell_line("4 4 45 heal Second Wind\n", &s)==1, "ok");
    check(s.level==4 && s.mp==4 && s.mag==45, "nums");
    check(strcmp(s.effect, "heal")==0, "effect");
    check(strcmp(s.name, "Second Wind")==0, "name spaces");
    check(parse_spell_line("nope\n", &s)==0, "bad");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("01-spell-line: all checks passed");
    return 0;
}
