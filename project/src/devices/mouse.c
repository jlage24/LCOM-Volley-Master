#include "mouse.h"
#include "../../../lab4/mouse.h"

MouseState mouse_state = {
    .dx = 0,
    .dy = 0,
    .left_button = false,
    .right_button = false,
    .middle_button = false,
    .new_data = false
};

static uint8_t mouse_buf[3];
static uint8_t mouse_idx = 0;

static int mouse_hook_id = 4;

int mouse_subscribe_int(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;

    *bit_no = mouse_hook_id;

    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != OK) {
        return 1;
    }

    return 0;
}

int mouse_unsubscribe_int(void) {
    if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
        return 1;
    }

    return 0;
}

void mouse_handle_bytes(uint8_t byte) {
    if (get_mouse_error()) {
        mouse_idx = 0;
        return;
    }

    if (mouse_idx == 0 && !(byte & MOUSE_SYNC_BIT)) {
        return;
    }

    mouse_buf[mouse_idx++] = byte;

    if (mouse_idx == 3) {
        struct packet pp;

        mouse_parse_packet(mouse_buf, &pp);

        mouse_state.dx = pp.delta_x;
        mouse_state.dy = pp.delta_y;
        mouse_state.left_button = pp.lb;
        mouse_state.right_button = pp.rb;
        mouse_state.middle_button = pp.mb;
        mouse_state.new_data = true;

        mouse_idx = 0;
    }
}

void mouse_clear_state(void) {
    mouse_state.dx = 0;
    mouse_state.dy = 0;
    mouse_state.new_data = false;
}
