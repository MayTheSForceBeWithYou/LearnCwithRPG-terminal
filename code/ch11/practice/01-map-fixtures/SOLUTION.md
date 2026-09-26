# Solution notes — 01-map-fixtures

See solutions/01_header.c, solutions/02_row_width.c, and
solutions/03_skip_comments.c for full programs.

## Approaches

### parse_header
Use sscanf for two ints. Reject if the conversion count is not 2, or if
width/height are not positive.

### row_ok
Trim newline. Length must equal expect. Every char must be a valid tile
(# or . in this drill — match the chapter is_valid_tile idea).

### count_data_lines
Walk line by line. Skip lines whose first non-space character is ;.
Count the rest. The sample has header + two map rows = 3 data lines.

## Fixture matrix vs exercise 1

| Fixture | Failure shape |
|---------|----------------|
| bad_header.map | no width height |
| short_row.map | row too short |
| invalid_tile.map | illegal glyph |
| truncated.map | fewer rows than height |
| absurd_height.map | unreasonable dimension |
| ok.map / comments.map | happy path / comments |

Do **not** leave assets/maps/overworld.map corrupted. Practice here.
