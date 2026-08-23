#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

#define CONFIG_MAX_LINE 256

void config_defaults(Config *config)
{
    config->encounters = 1;
    config->enemy_damage_pct = 100;
    config->xp_rate_pct = 100;
    config->gold_loss_pct = 0;
}

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                       s[len - 1] == ' '  || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }

    return s;
}

/* Accepts on/off/true/false/yes/no/1/0. Returns -1 if unrecognised. */
static int parse_bool(const char *value)
{
    if (strcmp(value, "on") == 0 || strcmp(value, "true") == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
        return 1;
    }
    if (strcmp(value, "off") == 0 || strcmp(value, "false") == 0 ||
        strcmp(value, "no") == 0 || strcmp(value, "0") == 0) {
        return 0;
    }

    return -1;
}

/* Returns 1 and writes *out if value is an integer within [low, high]. */
static int parse_percent(const char *value, int low, int high, int *out)
{
    char *end = NULL;
    long parsed = strtol(value, &end, 10);

    if (end == value || *end != '\0') {
        return 0;
    }
    if (parsed < low || parsed > high) {
        return 0;
    }

    *out = (int)parsed;
    return 1;
}

int config_load(Config *config, const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        /* Not an error: playing without a config file is normal. */
        return 0;
    }

    char line[CONFIG_MAX_LINE];
    int line_number = 0;

    while (fgets(line, sizeof line, f) != NULL) {
        line_number++;

        char *text = trim(line);

        if (text[0] == '\0' || text[0] == '#') {
            continue;   /* blank line or comment */
        }

        char *equals = strchr(text, '=');
        if (equals == NULL) {
            fprintf(stderr, "%s:%d: expected key = value\n", path, line_number);
            continue;
        }

        *equals = '\0';
        char *key = trim(text);
        char *value = trim(equals + 1);

        if (strcmp(key, "encounters") == 0) {
            int on = parse_bool(value);
            if (on < 0) {
                fprintf(stderr, "%s:%d: encounters must be on or off\n",
                        path, line_number);
            } else {
                config->encounters = on;
            }
        } else if (strcmp(key, "enemy_damage") == 0) {
            if (!parse_percent(value, 1, 1000, &config->enemy_damage_pct)) {
                fprintf(stderr, "%s:%d: enemy_damage must be 1-1000\n",
                        path, line_number);
            }
        } else if (strcmp(key, "xp_rate") == 0) {
            if (!parse_percent(value, 1, 1000, &config->xp_rate_pct)) {
                fprintf(stderr, "%s:%d: xp_rate must be 1-1000\n",
                        path, line_number);
            }
        } else if (strcmp(key, "gold_loss_on_death") == 0) {
            if (!parse_percent(value, 0, 100, &config->gold_loss_pct)) {
                fprintf(stderr, "%s:%d: gold_loss_on_death must be 0-100\n",
                        path, line_number);
            }
        } else {
            fprintf(stderr, "%s:%d: unknown setting '%s'\n",
                    path, line_number, key);
        }
    }

    fclose(f);
    return 1;
}
