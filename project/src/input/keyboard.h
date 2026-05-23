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

/* Keyboard state exposed to game_update() */
typedef struct {
    bool move_left;
    bool move_right;
    bool jump;
    bool pause;
} KeyboardState;

/* Called from main.c on every keyboard interrupt */
void keyboard_handle(uint8_t scancode);

/* game_update() reads this to know what Player 1 is doing */
const KeyboardState *get_keyboard_state(void);

/* Reset jump/pause after game_update() consumes them */
void keyboard_clear_events(void);

#endif /* _KEYBOARD_H_ */
