#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

/* Scancodes for Player 1 */
#define SCANCODE_A      0x1E
#define SCANCODE_D      0x20
#define SCANCODE_W      0x11
#define SCANCODE_ESC    0x01
#define SCANCODE_ENTER  0x1C

typedef struct {
    bool move_left;
    bool move_right;
    bool jump;
    bool pause;
    bool enter;
} KeyboardState;

/* Called on each keyboard interrupt. */
void keyboard_handle(uint8_t scancode);

const KeyboardState *get_keyboard_state(void);

/* Reset one-shot key events after the frame uses them. */
void keyboard_clear_events(void);

#endif /* _KEYBOARD_H_ */
