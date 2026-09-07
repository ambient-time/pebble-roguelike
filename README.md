# Dungeon Loop

A tiny adventurer meets a new dungeon creature every minute. Count the lit
braziers for the hour. Read the glowing floor stones for the minute: each full
row is ten, followed by the stones lit in the next row. Twelve burning braziers
means twelve o'clock; no glowing floor stones means minute zero.

The seconds move the encounter forward. The hero enters during seconds 00–06,
fights during 07–41, finishes the creature during 42–48, collects treasure during
49–54, and leaves during 55–59. These scenes show the minute's progress; they
are not an exact seconds display. A new chamber encounter starts at second zero.

This is an autonomous watchface, with no buttons, score, health bars, numerical
time plaque, or secondary digital clock. Its local 12-hour time encoding keeps
the braziers and runes from Luke Steuber's Pixel Dungeon source. The minute's
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
retains its minute-long encounter, hour braziers, minute runes, and enter/fight/
loot/exit sequence. The giant clock tablet, text HUD, persistent leveling,
procedural sparks, and subsecond browser animation are omitted.
