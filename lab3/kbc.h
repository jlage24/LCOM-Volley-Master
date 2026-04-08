#ifndef _LCOM_KBC_H_
#define _LCOM_KBC_H_

#include <lcom/lcf.h>
#include <stdint.h>

/* KBC Ports */
#define KBC_STATUS_REG  0x64
#define KBC_OUT_BUF     0x60

/* Status Register Bits */
#define KBC_OBF         BIT(0)   // Output Buffer Full
#define KBC_IBF         BIT(1)   // Input Buffer Full
#define KBC_PARITY_ERR  BIT(7)   // Parity Error
#define KBC_TIMEOUT_ERR BIT(6)   // Timeout Error

/* KBC IRQ */
#define KBC_IRQ         1

/* Scancodes */
#define ESC_BREAK       0x81     // Breakcode of ESC key
#define TWO_BYTE_PREFIX 0xE0     // Prefix for 2-byte scancodes

/* Polling Constants */
#define DELAY_US        20000
#define MAX_RETRIES     10

/* KBC Commands */
#define KBC_READ_CMD    0x20
#define KBC_WRITE_CMD   0x60

/* Command Byte Bits */
#define KBC_INT_KBD     BIT(0)  // Enable Keyboard Interrupts

void (kbc_ih)();
int kbc_write_cmd(int port, uint8_t cmd);
int kbc_read_out_buf(uint8_t *data);
uint8_t get_scancode();
bool get_kbc_error();
bool get_scancode_ready(); // NEW: Garante que não lemos lixo!

#endif
