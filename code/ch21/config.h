#ifndef CONFIG_H
#define CONFIG_H

/* Difficulty and comfort settings, from Checkpoint C.

   Every field has a documented default, so a missing, partial, or
   malformed config file is never fatal -- unknown keys are reported and
   skipped, and anything absent keeps its default. */
typedef struct {
    int encounters;          /* 1 = on, 0 = off        (default 1)   */
    int enemy_damage_pct;    /* percent multiplier     (default 100) */
    int xp_rate_pct;         /* percent multiplier     (default 100) */
    int gold_loss_pct;       /* percent lost on defeat (default 0)   */
} Config;

void config_defaults(Config *config);

/* Reads path over the top of the current values. Returns 1 if the file
   was read (even with warnings), 0 if it could not be opened -- in which
   case config is left holding whatever it had. */
int config_load(Config *config, const char *path);

#endif
