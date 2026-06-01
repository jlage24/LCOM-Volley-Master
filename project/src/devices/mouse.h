#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>

#define MOUSE_IRQ 12
#define MOUSE_SYNC_BIT BIT(3)

typedef struct {
    bool left_button;
    bool right_button;
    bool scroll_up;
    bool new_data;
} MouseState;

extern MouseState mouse_state;

int mouse_subscribe_int(uint8_t *bit_no);

int mouse_unsubscribe_int(void);

void mouse_handle_bytes(uint8_t byte);

#endif