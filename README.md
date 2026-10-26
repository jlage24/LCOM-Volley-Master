# 🏐 Volley Master

A two-player arcade volleyball game for **Minix**, built as the final project for **LCOM** (Laboratório de Computadores) at FEUP. One player uses the keyboard, the other uses the mouse, both at the same time.

Inspired by *Head Soccer*, the game puts two players on opposite sides of a court, each trying to make the ball land on the other's side.

| Main Menu                                                                       | Gameplay                                                                      |
| ------------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| <img width="1264" height="934" alt="main-menu" src="https://github.com/user-attachments/assets/ce68f5c3-a7e5-45fe-ad01-8e278f0bb412" />| <img width="1264" height="940" alt="gameplay" src="https://github.com/user-attachments/assets/476fce2f-1301-48fe-9d4f-51d49eeaaaaf" /> |

## Features

- **Dual input:** Player 1 uses the keyboard and Player 2 uses the mouse, concurrently, through two independent input pipelines.
- **Character selection:** a pool of six characters. Player 1 picks first and that character is locked for Player 2.
- **Field selection:** four environments (Beach, Desert, Ice, Jungle).
- **Match flow:** 3 → 2 → 1 → START countdown, 60-second timed matches, and **sudden death** on ties.
- **Menus:** Main Menu, Pause Menu, How To Play and Victory screens. Every menu works with keyboard and mouse.
- **Custom 2D physics:** gravity, hitbox-based ball–player rebounds, and a net that distinguishes side and top impacts.
- **Real-Time Clock:** current date and time displayed in the UI.

## Controls

| Action | Player 1 (Keyboard) | Player 2 (Mouse) |
|---|---|---|
| Move | `A` / `D` | Move mouse horizontally |
| Jump | `W` | Left click |
| Menu navigation | Keyboard | Cursor |
| Confirm | `ENTER` | Left click |
| Pause / Back | `ESC` | – |

## Hardware Used

| Device | Role |
|---|---|
| **Timer** (60 Hz) | Drives the game loop, countdown, match timer and sudden death |
| **Keyboard (KBC)** | Player 1 controls and menu navigation |
| **Mouse (PS/2)** | Player 2 controls and menu navigation. 3-byte packets with sync-bit validation |
| **Graphics Card** | VBE mode `0x118` (1024×768) with double buffering |
| **RTC** | Date and time shown in the UI |

## Architecture

The project is split into three layers (`devices`, `game`, `ui`), coordinated by `proj.c`, which handles initialization, device setup, the main interrupt loop and cleanup. XPM assets live in `images`.

```
project/
├── proj.c        # Init, main interrupt loop (driver_receive), cleanup
├── proj.h
├── Makefile
└── src/
    ├── devices/  # Hardware drivers: keyboard, mouse, RTC
    ├── game/     # Logic: game (state machine, score, match flow), player, ball
    ├── ui/       # Rendering: menus, sprites, HUD, asset management
    └── images/   # XPM assets (menus, characters, fields, victory screens)
```

**Design decisions**

- **Interrupt-driven loop:** the keyboard and mouse interrupts only write to input state structures. Physics, collisions, score updates and state transitions all run on Timer ticks (`game_update()` then `game_render()`), so gameplay speed doesn't depend on input frequency.
- **Central state machine:** it handles all transitions between menus and gameplay.
- **Double buffering:** frames are drawn to a back buffer and copied to VRAM only when complete, which avoids flickering.
- **XPM sprites with transparency:** characters can be mirrored horizontally, so one asset serves both facing directions.
- **Asset optimization:** palette reduction and indexed color conversion cut the assets from ~86 MB to ~10 MB, which improved compilation time and memory use under Minix.

## Building and Running

> Volley Master runs inside **Minix** (typically in a VM), not on a regular host OS.

```bash
cd project
make
lcom_run proj
```

## Authors

LCOM 2025/26 · Class 2LEIC15 · Group 5

| Name | Contact |
|---|---|
| David Pinto | up202404233@up.pt |
| João Lage | up202406458@up.pt |
| Luís Torrão | up202406471@up.pt |
| Mafalda Pacheco | up202406162@up.pt |

## License
All rights reserved. This project was developed for academic purposes at FEUP.
