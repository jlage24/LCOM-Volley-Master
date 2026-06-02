#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>

#define MOUSE_IRQ 12
#define MOUSE_SYNC_BIT BIT(3)

typedef struct {
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

void mouse_handle_bytes(uint8_t byte);
void mouse_clear_state(void);

#endif
