#include "keyboard.h"
#include "../../../lab3/kbc.h"

static KeyboardState kb_state = {false, false, false, false, false};

void keyboard_handle(uint8_t scancode) {
    bool make = !(scancode & BIT(7));

    switch (scancode & ~BIT(7)) {
        case SCANCODE_A:   kb_state.move_left  = make; break;
        case SCANCODE_D:   kb_state.move_right = make; break;
        case SCANCODE_W:   if (make) kb_state.jump  = true; break;
        case SCANCODE_ESC: if (make) kb_state.pause = true; break;
        case SCANCODE_ENTER: if (make) kb_state.enter = true; break;
        default: break;
    }
}

const KeyboardState *get_keyboard_state(void) {
    return &kb_state;
}

void keyboard_clear_events(void) {
    kb_state.jump  = false;
    kb_state.pause = false;
    kb_state.enter = false;
}
