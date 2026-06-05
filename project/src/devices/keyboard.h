#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/*
 * Keyboard state interface.
 *
 * keyboard_handle translates raw scancodes into held movement state plus
 * one-frame navigation/action events consumed by game_update.
 */
#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

/* Scancodes for Player 1 */
#define SCANCODE_A      0x1E
#define SCANCODE_D      0x20
#define SCANCODE_W      0x11
#define SCANCODE_S      0x1F
#define SCANCODE_ESC    0x01
#define SCANCODE_ENTER  0x1C

typedef struct {
    /* move_* are held inputs; the remaining fields are cleared each frame. */
    bool move_left;
    bool move_right;
    bool jump;
    bool pause;
    bool enter;
    bool up;
    bool down;
    bool left;
    bool right;
} KeyboardState;

/* Called on each keyboard interrupt. */
void keyboard_handle(uint8_t scancode);

/* Returns the current input snapshot owned by keyboard.c. */
const KeyboardState *get_keyboard_state(void);

/* Reset one-shot key events after the frame uses them. */
void keyboard_clear_events(void);

#endif /* _KEYBOARD_H_ */
