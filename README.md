# Dungeon Loop

A tiny adventurer meets a new dungeon creature every minute. Read the large
numbers over the chamber: FLOOR is the local hour from 00 to 23, and ROOM is
the minute from 00 to 59. FLOOR 17 / ROOM 29 means 17:29. The glowing floor
stones also retain the minute count: each full row is ten, followed by the
stones lit in the next row.

The seconds move the encounter forward. The hero enters during seconds 00–06,
fights during 07–41, finishes the creature during 42–48, collects treasure during
49–54, and leaves during 55–59. These scenes show the minute's progress; they
are not an exact seconds display. A new chamber encounter starts at second zero.

This is an autonomous watchface, with no buttons, score or health bars. The
owner-requested floor and room numbers replace the counted hour braziers to
make time easier to read. The runes remain from Luke Steuber's Pixel Dungeon source. The minute's
creature is determined from the day and local hour/minute, so opening the face
mid-minute resumes that encounter immediately. There is no saved game, loot
inventory, network, or catch-up simulation.

## Rendering and targets

Basalt, Diorite, Emery, and Flint only. Every drawn scene pixel occupies exactly
2 × 2 device pixels. Emery shows extra chamber space around the same sprites;
there is no unequal horizontal/vertical stretching. All shapes use pixel
rectangles. Color uses warm fire, cyan runes, and distinct creature colors;
monochrome uses flame/stone silhouettes to distinguish lit and unlit counts.
The face requests one redraw each second and pauses drawing when covered.
Physical-watch battery use and readability have not been measured.

## Build and validation

```sh
make test
pebble build --sdk 4.33.1
```

The sanitizer-backed host test covers all 86,400 states of a day: hour/minute
encoding, phase boundaries, bounded hero positions, stable encounters within
a minute, and new encounters at rollovers. Native logs include
`Dungeon time=HH:MM:SS kind=N phase=N heap=N` for capture verification.
Emulator captures and natural noon/midnight rollover checks passed on all four
targets. Physical testing remains pending.

## Source

Copyright 2026 Luke Steuber. MIT License.

The complete supplied [Pixel Dungeon source](https://datapoems.io/clocks/pixel-dungeon/)
is preserved in `reference/source.html` with a checksum. This native adaptation
retains its minute-long encounter, minute runes, and enter/fight/loot/exit
sequence. Version 0.1.1 replaces the braziers with the requested floor/hour and
room/minute numbers. The giant clock tablet, unrelated text HUD, persistent leveling,
procedural sparks, and subsecond browser animation are omitted.
