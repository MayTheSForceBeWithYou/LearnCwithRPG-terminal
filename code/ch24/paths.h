#ifndef PATHS_H
#define PATHS_H

#include <stddef.h>

/* Where the game's files live at runtime.

   Until now every path in this program was relative to the working
   directory -- "assets/levels.txt" -- which works perfectly right up to
   the moment somebody runs the game from anywhere else. An installed
   program does not get to choose its working directory.

   Two different questions, with two different answers:

     - Data files are READ-ONLY and shipped with the game. They may sit
       in the build tree, or in /usr/local/share once installed.
     - The save file is WRITTEN, so it cannot live beside the data at
       all: /usr/local/share is not writable by a player. It belongs in
       the user's own directory. */

/* Writes the full path of a shipped data file into out. relative is
   given without a leading directory -- "levels.txt", "maps/mudwick.map".
   Always produces something; falls back to the relative path itself. */
void paths_data(char *out, size_t out_size, const char *relative);

/* Writes the full path of the save file into out, creating the
   directory that holds it if necessary. Returns 0 (and still fills out
   with a usable fallback) if that directory could not be created. */
int paths_save(char *out, size_t out_size);

/* The directory data files were found in, for the "where am I reading
   from?" line the game prints on request. */
const char *paths_data_dir(void);

#endif
