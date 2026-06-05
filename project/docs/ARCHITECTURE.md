# Architecture

This document summarizes the main architecture of Volley Master, project made for LCOM, and explains
how the game loop, state machine, input, rendering, physics, RTC and assets work
together.

## Game Loop

The entry point is `proj_main_loop` in `proj.c`. It initializes the timer,
graphics mode, back buffer, assets, game state, keyboard and mouse, then enters
the Minix interrupt loop.

The timer is configured to 60 Hz and acts as the heartbeat of the game. Game
logic advances only when the timer interrupt arrives:

- `game_update()` consumes the latest input state and advances one frame.
- `game_render()` draws the current frame if the game is still running.
- `keyboard_clear_events()` clears one-frame keyboard events.
- `mouse_clear_state()` clears per-frame mouse deltas.

Keyboard and mouse interrupts do not directly update gameplay. They only update
input state that will be consumed on the next timer tick. This keeps gameplay
speed deterministic and independent from the rate of input interrupts.

## State Machine

The central state machine is implemented in `src/game/game.c` and uses the
`GameState` enum from `src/game/game.h`.

The main states are:

- `STATE_MENU`: main menu with Start, Exit and How To Play.
- `STATE_CHARACTER_SELECT`: two-step character selection, first Player 1 and
  then Player 2.
- `STATE_FIELD_SELECT`: field/background selection.
- `STATE_COUNTDOWN`: pre-match countdown where gameplay is frozen.
- `STATE_PLAYING`: active match state.
- `STATE_PAUSED`: pause menu drawn over the current rally.
- `STATE_END`: winner screen with replay/menu options.
- `STATE_HOW_TO_PLAY`: instructions screen, opened from menu or pause.

`game_update()` is responsible for state transitions. It handles keyboard and
mouse navigation, starts matches after selections, pauses/resumes play, and ends
the match when a winner is determined.

The countdown is frame-based: each number lasts a fixed number of timer ticks.
After the countdown finishes, the state changes to `STATE_PLAYING`.

The match timer is also frame-based. Every 60 timer frames, one game second is
subtracted. If time reaches zero and the score is tied, the game enters sudden
death. In sudden death, the next scored point immediately wins the match.

## Input System

Input is split into low-level interrupt handling and game-friendly state.

Keyboard input is handled by `src/devices/keyboard.c`. Raw scancodes are
translated into a `KeyboardState`:

- `move_left` and `move_right` are held inputs used by Player 1.
- `jump`, `pause`, `enter`, `up`, `down`, `left` and `right` are one-frame
  events used for actions and menu navigation.

The game clears one-frame events after each timer update, so pressing a key
triggers one state-machine action instead of repeating forever.

Mouse input is handled by `src/devices/mouse.c`. The mouse interrupt provides
one byte at a time, so the module assembles complete three-byte PS/2 packets.
The first byte must contain the sync bit; otherwise the byte is ignored to avoid
misaligned packets.

After a complete packet is parsed, `mouse_state` is updated:

- `dx` and `dy` are per-frame movement deltas.
- button fields represent the latest known button state.

The mouse is used for menu cursor movement and for Player 2 movement/jump.

## Rendering System

Rendering is split between state-level drawing and low-level drawing helpers.

`game_render()` in `src/game/game.c` selects what to draw based on the current
state. Each branch draws a complete frame and calls `video_flush()` once at the
end.

`src/ui/game_render.c` owns the low-level rendering pipeline:

- a software back buffer is allocated in RAM;
- all pixels, rectangles, text and sprites are drawn into that buffer;
- `video_flush()` copies the finished frame to VRAM.

This avoids drawing partially updated frames directly to video memory.

Sprites are stored as XPM images. `draw_xpm()` respects the XPM transparency
color, so sprites can be layered over backgrounds. Player sprites can also be
drawn flipped horizontally, which allows the same asset to be reused for both
facing directions.

Menus and static screens are drawn in `src/ui/menu.c`. That module only composes
backgrounds and buttons; navigation and state transitions remain in `game.c`.

## Physics

Physics is updated once per timer frame while the game is in `STATE_PLAYING`.

Players are updated in `src/game/player.c`:

- Player 1 uses keyboard movement and jump.
- Player 2 uses mouse horizontal movement and left-button jump.
- both players are constrained to their side of the court;
- gravity affects vertical movement until the player lands on the floor.

The ball is updated in `src/game/ball.c`:

- gravity changes vertical velocity;
- velocity changes position;
- wall and ceiling collisions keep the ball inside the arena;
- floor contact ends the rally and reports which side lost the point.

Ball-player collision uses circle-vs-rectangle logic. The closest point on the
player hitbox is found, the ball is pushed out of overlap, and the rebound
velocity is calculated from where the ball hit the player. Center hits produce a
more vertical bounce, while edge hits produce stronger horizontal movement.
Player velocity is partially transferred to the ball, so movement and jumping
influence the shot.

Ball-net collision also uses circle-vs-rectangle logic. Top collisions are
handled differently from side collisions so the ball can bounce on top of the
net without being reflected sideways. Side collisions push the ball out and
reverse horizontal velocity.

Scoring is handled by `game.c` after `ball_update()` reports a floor hit. A hit
on the left side awards Player 2; a hit on the right side awards Player 1.
Outside sudden death, a round reset keeps score and timer. In sudden death, the
first point ends the match.

## RTC

The RTC code is in `src/devices/rtc.c`. It reads the current date and time from
the hardware RTC and converts it into the `rtc_date` structure defined in
`src/devices/rtc.h`.

Before reading values, the code waits until the RTC is not updating. This avoids
mixing values from two different seconds. Register B is then used to determine
whether values are stored in BCD or binary format and whether the RTC is in
12-hour or 24-hour mode.

The game does not query the RTC every frame. `game.c` caches the value and
refreshes it only while menu-like states are visible, at roughly one update per
second. The cached date/time is rendered on the main menu by
`video_draw_rtc_date()`.

## Assets

Assets are loaded in `src/ui/assets.c` and declared in `src/ui/assets.h`.
They are bundled as XPM maps and converted to `xpm_image_t` objects during
startup by `load_all_assets()`.

The asset registry stores:

- menu backgrounds;
- buttons;
- character sprites;
- field backgrounds;
- winner screens;
- ball sprite;
- How To Play screen.

Field backgrounds are loaded from `src/ui/assets_fields.c`. They are separated
because field XPMs are large and keeping them in their own translation unit makes
the main asset loader easier to manage.

The selected player sprites are exposed through `player1_pixmap` and
`player2_pixmap`. During character selection, `game.c` updates these pointers to
match the chosen characters. Rendering code then draws the currently selected
sprites without needing to know about the selection process.

All loaded XPM byte buffers are released by `free_all_assets()` before the
program exits graphics mode.
