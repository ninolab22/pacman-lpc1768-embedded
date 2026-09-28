# Pac-Man — LandTiger (LPC1768)

A Pac-Man game for the **LandTiger** development board (NXP **LPC1768**, ARM Cortex-M3),
written in C and built with **Keil µVision**. It is the project for *Architetture dei Sistemi di
Elaborazione* — **Extra Point #1** (core game) and **Extra Point #2** (ghost AI, sound, CAN).

The code uses a clean architecture: a **logical grid model is the single source of truth** for the
game, and the screen is just a projection of that model. No game logic ever reads pixels back from the
display — this makes movement, collisions and scoring robust.

---

## The game

Pac-Man moves through a maze eating pills to score points while a ghost (Blinky) chases him and a
60‑second countdown runs. Eating a Power Pill makes the ghost vulnerable for a while. Clear the maze to
win; run out of time or lives to lose.

- **Standard pill**: +10 points
- **Power pill**: +50 points, and turns the ghost *frightened* (blue) for 10 s
- **Eating a frightened ghost**: +100 points (the ghost respawns after 3 s at the centre)
- **Extra life**: every 1000 points (you start with 1 life)
- **Touching the ghost while it chases you**: you lose a life
- **Win**: all 240 pills eaten → *VICTORY!*
- **Lose**: countdown reaches 0, or lives reach 0 → *GAME OVER*

### Controls

| Input | Action |
|---|---|
| **Joystick** (up/down/left/right) | Choose Pac-Man's direction. He keeps going until he hits a wall or you choose a new direction. |
| **INT0** button | Pause / resume. **The game starts paused** — press INT0 to begin. |

The left/right **tunnel** teleports Pac-Man from one side of the maze to the other.

---

## Implemented requirements

### Extra Point #1 (core game)

| Spec | Description | Status |
|---|---|---|
| 1 | Maze filled with exactly **240 standard pills**, central box preserved | Implemented (count verified) |
| 2 | **6 power pills** appearing at **random position and time** (replace a standard pill) | Implemented |
| 3 | **Joystick** movement: continuous until a new direction or a wall (then stops) | Implemented |
| 4 | **Teleport** tunnels (exit one side, re-enter the other, keeping direction) | Implemented |
| 5 | Eating pills: **+10** (standard) / **+50** (power) | Implemented |
| 6 | **Extra life every 1000 points** (starting from 1) | Implemented |
| 7 | **INT0 pause** with centred "PAUSE" message; **game starts paused** | Implemented |
| 8 | **Countdown** from 60 s to 0 (frozen while paused) | Implemented |
| 9 | **Victory** screen when all pills are eaten | Implemented |
| 10 | **Game Over** screen when the countdown expires | Implemented |
| HUD | Current **Score**, **Remaining Lives**, **Countdown** on the LCD | Implemented |

### Extra Point #2 (extensions — require the physical board)

| Spec | Description | Status |
|---|---|---|
| 1 | **AI ghost (Blinky)** that chases Pac-Man; contact costs a life. **Chase** and **Frightened** modes; frightened lasts 10 s, ghost is edible for +100, then respawns after 3 s at the centre. Ghost gets faster as the game progresses. | Implemented |
| 2 | **Speaker**: background music + sound effects | Implemented (DAC + Timer1) |
| 3 | **CAN** bus (external loopback CAN1→CAN2) transmitting Score, Lives and Countdown in a 32‑bit message | Implemented |

**Ghost AI.** The ghost uses a **BFS distance field** computed from Pac-Man's cell (equivalent to A*
with a null heuristic on an unweighted grid, so it always follows a shortest path). In *Chase* it steps
toward the minimum distance; in *Frightened* it steps toward the maximum distance (it flees).

**CAN message format** (saved in a single `uint32_t`):

```
[ Remaining time : 8 bits ][ Remaining lives : 8 bits ][ Score : 16 bits ]
msg = (time << 24) | (lives << 16) | (score & 0xFFFF);
```

CAN1 transmits and CAN2 receives every second (external loopback: connect CANH/CANL of CAN1 and CAN2
on the board's connector).

---

## Source layout

```
Source/
  sample.c          Boot + main game loop (scheduler)
  maze.c / maze.h   Static maze layout -> logical grid[][] (240 pills), tunnels, spawn
  game.c / game.h   Game state and rules (movement, eating, lives, win/lose, contact)
  ghost.c / ghost.h Ghost AI (BFS), Chase/Frightened/Eaten, respawn, speed
  render.c/render.h Drawing from the model (cells, Pac-Man, ghost, HUD, screens)
  audio.c / audio.h Speaker: DAC (P0.26) + Timer1, music + SFX sequencer
  can.c / can.h     CAN1/CAN2 loopback driver + 32-bit status message
  RIT/IRQ_RIT.c     50 ms tick: joystick, INT0 debounce, audio sequencer
  timer/IRQ_timer.c Timer0 (1 s countdown + CAN trigger), Timer1 (audio tone)
  button_EXINT/...  INT0 (pause)
  GLCD/, joystick/, led/, adc/, CMSIS_core/, TouchPanel/   board drivers (reused)
```

### How it runs (scheduler)

- **RIT (50 ms)** — game tick: samples the joystick, debounces INT0, advances the audio sequencer.
- **Main loop** — moves Pac-Man and the ghost at their own cadences, spawns power pills, draws the
  pause / end screens, and sends the CAN message once per second.
- **Timer0 (1 s)** — countdown + ghost timers; flags the per-second CAN send.
- **Timer1** — generates the speaker square-wave tone.

Everything advances only in the `RUNNING` state, so pausing freezes the whole game.

---

## Build & run

Open `sample.uvprojx` in Keil µVision. Two targets are provided:

- **SW_Debug** — LandTiger software emulator (good for the core EP1 gameplay).
- **LandTiger_LPC1768 (release)** — the **physical board** (required for EP2: speaker and CAN).

Build the target, then start debug/run.

> Save the project with all compilation options before zipping the deliverable
> (`extrapoint1.zip` / `extrapoint2.zip`).

### Audio (why you might not hear music)

1. The game **starts paused** — press **INT0** to start; music begins automatically while playing.
2. Sound comes from the **DAC → speaker on the physical board**. In the emulator you generally won't
   hear it; raise **Speaker volume** in the LandTiger *Settings* dialog, or watch *Peripherals → D/A
   Converter* to see the output toggling. EP2 is meant to be tested on the board.

### CAN (external loopback)

Connect **CANH/CANL of CAN1 to CAN2** on the board connector. CAN1 sends, CAN2 receives. The bit-timing
value (`BTR`) in `can.c` may need adjusting to your board's CAN clock if the baud rate doesn't match.

---

## Configurable parameters

Defined in `game.h` (and `audio.c` / `can.c`):

| Define | Meaning |
|---|---|
| `MOVE_TICKS` | Pac-Man speed (cells per N RIT ticks) |
| `START_LIVES`, `START_TIME` | initial lives (1) and countdown (60 s) |
| `EXTRA_LIFE_STEP` | points per extra life (1000) |
| `POWER_MAX`, `SPAWN_MIN/MAX` | power-pill count (6) and random timing |
| `FRIGHT_SECS`, `EATEN_SECS` (ghost.h) | frightened (10 s) and respawn (3 s) durations |

---

## Notes

- The maze is authored as a character map in `maze.c` and converted to the grid at start-up; the pill
  count is verified to be exactly **240**.
- The original sample's old, pixel-based game code is **excluded from the build** (a few legacy
  functions remain inside `#if 0 … #endif` in `GLCD.c` only because they contain legacy non‑ASCII
  characters; they are not compiled and can be deleted in the editor if desired). The low-level LCD
  driver in `GLCD.c` is reused unchanged.
