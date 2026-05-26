#ifndef _LCOM_MOUSE_H_
#define _LCOM_MOUSE_H_

#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>

/* Mouse IRQ Line */
#define MOUSE_IRQ           12

/* KBC Commands for Mouse */
#define KBC_WRITE_TO_MOUSE  0xD4

/* Mouse Commands */
#define MOUSE_ENABLE_DATA   0xF4
#define MOUSE_DISABLE_DATA  0xF5

/* Mouse Acknowledgement Bytes */
#define MOUSE_ACK           0xFA    // Acknowledged
#define MOUSE_NACK          0xFE    // Invalid Byte (Resend)
#define MOUSE_ERROR         0xFC    // Second consecutive invalid byte

/* Mouse Packet Bit Masks (Byte 1) */
#define MOUSE_LB            BIT(0)  // Left Button
#define MOUSE_RB            BIT(1)  // Right Button
#define MOUSE_MB            BIT(2)  // Middle Button
#define MOUSE_SYNC_BIT      BIT(3)  // Bit 3 is always 1 in the first byte
#define MOUSE_X_SIGN        BIT(4)  // X Sign (1 if negative)
#define MOUSE_Y_SIGN        BIT(5)  // Y Sign (1 if negative)
#define MOUSE_X_OVF         BIT(6)  // X Overflow
#define MOUSE_Y_OVF         BIT(7)  // Y Overflow

typedef struct {
    int16_t dx;
    bool jump;
    bool new_data;
} MouseState;

extern MouseState mouse_state;

void mouse_handle_bytes(uint8_t byte);

void (mouse_ih)();
uint8_t get_mouse_byte();
bool get_mouse_error();
int mouse_write_cmd(uint8_t cmd);
void mouse_parse_packet(uint8_t *bytes, struct packet *pp);

#endif
