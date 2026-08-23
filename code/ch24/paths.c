#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "paths.h"

/* Where the installed copy keeps its data. The Makefile passes this in
   with -DDATADIR=... so one build can be installed anywhere; the
   fallback here only matters if someone compiles a file by hand. */
#ifndef DATADIR
#define DATADIR "/usr/local/share/some-assembly-required"
#endif

#define PATH_MAX_LEN 512

static char data_dir[PATH_MAX_LEN];
static int  data_dir_ready = 0;

static int directory_exists(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        return 0;
    }

    /* S_ISDIR is a macro from <sys/stat.h>; a file named "assets" is not
       a data directory, and checking existence alone would accept one. */
    return S_ISDIR(st.st_mode);
}

/* Resolution order, most specific first:

     1. $SAR_DATA_DIR   -- an explicit override, which makes the game
                           testable without installing it anywhere.
     2. ./assets        -- running from the build tree, as you have been
                           doing for twenty-four chapters.
     3. DATADIR         -- baked in at compile time; the installed case.

   Falling through all three still yields DATADIR, so callers always get
   a path and the error surfaces as "cannot open", which the loaders
   already report properly. */
static void resolve_data_dir(void)
{
    const char *override = getenv("SAR_DATA_DIR");

    if (override != NULL && override[0] != '\0') {
        snprintf(data_dir, sizeof data_dir, "%s", override);
    } else if (directory_exists("assets")) {
        snprintf(data_dir, sizeof data_dir, "assets");
    } else {
        snprintf(data_dir, sizeof data_dir, "%s", DATADIR);
    }

    data_dir_ready = 1;
}

const char *paths_data_dir(void)
{
    if (!data_dir_ready) {
        resolve_data_dir();
    }

    return data_dir;
}

void paths_data(char *out, size_t out_size, const char *relative)
{
    snprintf(out, out_size, "%s/%s", paths_data_dir(), relative);
}

/* mkdir, but "it already exists" is success rather than failure. */
static int ensure_directory(const char *path)
{
    if (directory_exists(path)) {
        return 1;
    }

    /* 0755: the owner may write, everyone may look. */
    return mkdir(path, 0755) == 0;
}

int paths_save(char *out, size_t out_size)
{
    char dir[PATH_MAX_LEN];
    const char *xdg = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");

    /* The XDG Base Directory spec says user data goes in
       $XDG_DATA_HOME, defaulting to $HOME/.local/share. Following it
       means the save turns up where a Linux user expects it. */
    if (xdg != NULL && xdg[0] != '\0') {
        snprintf(dir, sizeof dir, "%s/some-assembly-required", xdg);
    } else if (home != NULL && home[0] != '\0') {
        char local[PATH_MAX_LEN];

        snprintf(local, sizeof local, "%s/.local", home);
        ensure_directory(local);
        snprintf(local, sizeof local, "%s/.local/share", home);
        ensure_directory(local);

        snprintf(dir, sizeof dir, "%s/.local/share/some-assembly-required",
                 home);
    } else {
        /* No HOME at all is strange but survivable: save beside the
           player rather than refusing to save. */
        snprintf(out, out_size, "save.txt");
        return 0;
    }

    if (!ensure_directory(dir)) {
        snprintf(out, out_size, "save.txt");
        return 0;
    }

    snprintf(out, out_size, "%s/save.txt", dir);
    return 1;
}
