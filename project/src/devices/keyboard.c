/*
 * Keyboard input adapter.
 *
 * The low-level KBC interrupt handler reads raw scancodes; this module turns
 * them into game-friendly state. Held movement keys remain true until their
 * break code arrives, while menu/action events are cleared once per game tick.
 */
#include "keyboard.h"
#include "../../../lab3/kbc.h"

static KeyboardState kb_state = {
    .up = false,
    .down = false,
    .left = false,
    .right = false,
    .enter = false,
    .pause = false,
    .move_left = false,
    .move_right = false,
    .jump = false
};

void keyboard_handle(uint8_t scancode) {
    /* Bit 7 distinguishes make codes from break codes for these one-byte keys. */
    bool make = !(scancode & BIT(7));

    switch (scancode & ~BIT(7)) {
        case SCANCODE_A:
            kb_state.move_left = make;
            /* Direction events are edge-like menu inputs; movement is held state. */
            if (make) kb_state.left = true;
            break;
        case SCANCODE_D:
            kb_state.move_right = make;
            if (make) kb_state.right = true;
            break;
        case SCANCODE_W:
            if (make) {
                kb_state.jump = true;
                kb_state.up = true;
            }
            break;
        case SCANCODE_S:
            if (make) kb_state.down = true;
            break;
        case SCANCODE_ESC: if (make) kb_state.pause = true; break;
        case SCANCODE_ENTER: if (make) kb_state.enter = true; break;
        default: break;
    }
}

const KeyboardState *get_keyboard_state(void) {
    return &kb_state;
}

void keyboard_clear_events(void) {
    /* Clear one-frame events after game_update consumes them. */
    kb_state.jump  = false;
    kb_state.pause = false;
    kb_state.enter = false;
    kb_state.up = false;
    kb_state.down = false;
    kb_state.left = false;
    kb_state.right = false;
}
