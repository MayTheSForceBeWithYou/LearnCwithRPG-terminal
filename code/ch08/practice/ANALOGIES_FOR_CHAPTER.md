# Analogies for Chapter 8 — game loop, glyphs, render seam

Fold into chapter prose or LESSON intros as needed. Use **RPG**, not the J-prefixed genre tag.

## Opaque glyph table

**Two-column menu board.** The kitchen (game logic) orders "wall" or "floor";
the board (renderer) decides which chalk marks appear. Changing wall chalk
from `#` to `%` is one board edit — you do not rewrite every order ticket.

**Rejected wrong reading:** scattering `'#'` literals into `draw_world` is
like writing the chalk mark on every ticket. One style change means hunting
every ticket.

## Present / refresh boundary

**Kitchen ticket window.** Game code slips a finished frame through the
window (`render_present`). The cook behind the window (`refresh` /
backend flush) is not someone the dining room should know by name. If
waiters walk into the kitchen and flip the grill themselves, a new kitchen
(SDL backend) means retraining every waiter.

**Rejected wrong reading:** "`render_present` is one line, so delete it and
call `refresh` from `draw_world`." One-line wrappers exist so the *call
sites* stay stable when the implementation grows.

## Key dispatch table

**Switchboard vs hard-wired lamps.** Mapping `KEY_UP` and `'w'` onto the
same `INPUT_UP` action is one switchboard row. Wiring each lamp directly
to movement code duplicates the path and makes a second input source
(gamepad, demo replay) painful.

## Game loop cadence

**Stage manager cue sheet.** Input → update world → draw → present is one
cue cycle. Skipping present, or drawing mid-update from a random helper,
is like moving scenery while actors are mid-line — it works until the
stage gets bigger.
