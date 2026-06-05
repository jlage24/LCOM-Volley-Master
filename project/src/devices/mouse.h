#ifndef _MOUSE_H_
#define _MOUSE_H_

/*
 * Mouse state interface.
 *
 * The interrupt handler feeds raw bytes to mouse_handle_bytes, which assembles
 * packets and updates mouse_state for menu cursor movement and Player 2 input.
 */
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>

#define MOUSE_IRQ 12
#define MOUSE_SYNC_BIT BIT(3)

typedef struct {
    /* dx/dy are per-frame deltas; button fields represent the latest packet. */
    int16_t dx;
    int16_t dy;
    bool left_button;
    bool right_button;
    bool middle_button;
    bool new_data;
} MouseState;

extern MouseState mouse_state;

int mouse_subscribe_int(uint8_t *bit_no);
int mouse_unsubscribe_int(void);

/* Accepts one raw PS/2 byte and publishes state after a complete packet. */
void mouse_handle_bytes(uint8_t byte);
void mouse_clear_state(void);

#endif
