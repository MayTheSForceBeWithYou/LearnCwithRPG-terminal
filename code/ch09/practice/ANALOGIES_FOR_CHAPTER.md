# Analogies & alternate situations for Chapter 9

*Draft for Monk to fold into chapter prose. Voice: direct, warm, concrete.
Prefer the label RPG (not the J-prefixed genre tag). Do not paste these wholesale without editing for flow.*

---

## 1. Two rulers, one desk (coordinate spaces)

Imagine two rulers taped to your desk. One measures from the **map's**
top-left corner (world space). The other measures from the **window's**
top-left corner (screen space). Both mark "12," but they mean different
spots on the desk. The camera is the sticky note that says: "the window's
0 lines up with map position *this*." Subtract once, and you've converted.
Mix the rulers and nothing crashes — the `@` just quietly sits in the
wrong place, which is worse.

**Alternate situation:** A stage play. "Seat 12" in the building (world)
vs "seat 12 of the section currently lit" (screen). The spotlight crew
only care about the lit section; the usher cares about the building.

---

## 2. The photograph and the poster (why cameras exist)

The map is a giant poster. The terminal is a small photograph frame you
hold in front of it. You never shrink the poster; you slide the frame.
`camera.x` / `camera.y` are "where the frame's top-left corner touches the
poster." Drawing is: for each cell in the frame, ask what's on the poster
underneath, and put that ink on the photo.

Without a camera you're forced to either (a) print posters that fit the
frame (Chapter 8's tiny room) or (b) ask the player to imagine off-screen
tiles. Sliding the frame is how walking a bigger world stays interesting.

---

## 3. Centering a painting on a nail, then the wall stops you (clamp)

You want the player's face in the middle of the frame (`target - view/2`).
Near the poster's edge, that wish would hang the frame off the poster —
half the photo would show empty air (or, in C, memory that isn't your
map). Clamping is the wall bracket: "center if you can; if you can't,
pin the frame flush with the edge and let the subject walk toward the
frame's border instead."

That's why near a corner the `@` starts moving on screen again. The frame
stopped; the actor didn't.

**Numbers to keep in prose:** on a 40-wide map with a 20-wide view, the
rightmost legal frame position is `40 - 20 = 20`, not `40`. Off-by-one
here paints a phantom wall of `'#'` from `map_tile_at`'s bounds check —
a bug that looks like content.

---

## 4. Elevator floors vs walking the hallway (page-snap vs smooth follow)

Page-snap is an elevator: you ride until the doors open on a whole new
floor (screen). Integer `/` tells you which floor; `%` tells you where
you stand between doors. Smooth center-follow is walking the hallway —
every step slides the scenery a little.

Neither is "more correct." Zelda's overworld rooms chose the elevator;
most modern RPG overworlds choose the hallway. This course's game chooses
the hallway **and** keeps the wall bracket (clamp). Practice the elevator
in a side drill so you own the math without ripping out the hallway.

**Caution for exercise redesign:** asking learners to replace
`camera_center_on` with paging teaches `/`, but it also teaches "the game
camera is whatever I last pasted," which fights every later chapter.

---

## 5. A dog on a long leash (dead zone)

Dead-zone cameras are a dog on a leash. Inside the leash's slack, the dog
(`@`) trots around and the owner (camera) stands still. When the dog hits
the end of the leash, it *pulls* the owner along. The leash length is
your screen-space margins.

Critical difference from center-follow: the owner's feet remember where
they were. You can't compute "where should I stand?" from the dog alone;
you need last frame's owner position, then ask whether the leash went
taut. That's why exercise 4's sketch answer talks about state between
frames — `cam->x` / `cam->y` *are* that state, if you stop overwriting
them unconditionally.

---

## 6. A security guard who only watches the doorway (defensive map vs camera bug)

`map_tile_at` returning `'#'` for out-of-bounds coordinates is a security
guard who treats "outside the building" as "a locked door." Helpful —
unless you use it to ignore a camera that walked into the parking lot.
The guard makes the bug *look* like a wall. Clamp is locking the camera
crew's wheels so they never ask the guard that question by accident.

Good common-error anecdote: forgetting clamp, seeing a strip of `#`, and
"fixing the map data" for an hour.

---

## 7. Alternate situations (quick inserts)

- **Minimap vs viewport:** Minimap speaks world space; the big view speaks
  screen space. Same numbers, different rulers — label them.
- **UI chrome:** If you later reserve three rows for a status bar, the
  *viewport* shrinks. Clamp's upper bound uses `VIEW_HEIGHT`, not the
  full terminal height — the camera math shouldn't care about chrome.
- **Cutscene lock:** A boss intro might freeze `cam` and move entities in
  world space only. That's "stop updating the frame" — trivial if camera
  update is one function call in the loop.
- **Shake:** Add a temporary offset *after* clamp for screen shake; never
  bake shake into the clamped origin or you'll crawl along the map edge.

---

## 8. Suggested chapter touch-ups (Monk owns the merge)

- In the `/` `%` spotlight: keep the Zelda elevator analogy; add one line
  that paging is practiced in `code/ch09/practice/01-page-snap/`, not by
  overwriting the game camera.
- In exercise 3: retarget to "run the page-snap drill" (or make it
  open-ended compare), so solution text never says "delete `clamp`."
- In exercise 4: point at `02-dead-zone/` for learners who want to go
  beyond a sketch.
- Somewhere in "What just happened" or Common errors: one sentence from
  analogy 6 (guard vs clamp) — it sells why production keeps
  center+clamp even after you've met cooler variants.

---

## Skills these analogies support

| Analogy | Skill |
|---------|--------|
| Two rulers | World vs screen naming discipline |
| Poster + frame | Why a camera exists |
| Nail + wall bracket | Center then clamp |
| Elevator vs hallway | Page-snap vs smooth follow; drill vs production |
| Dog on a leash | Dead-zone statefulness |
| Security guard | Defensive accessors don't excuse camera bugs |
