#include <lcom/lcf.h>
#include "kbc.h"

static uint8_t scancode = 0; 
static bool kbc_error = false;
static bool scancode_ready = false; // checks if the scancode is fresh

void (kbc_ih)() {
    uint8_t status;
    kbc_error = false; // reset error flag for each interrupt
    scancode_ready = false;
    // Read the status register
    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) {
        kbc_error = true;
        return;
    }
    // Check if OBF is full (data is available)
    if (status & KBC_OBF) {
        // Read the Output Buffer
        if (util_sys_inb(KBC_OUT_BUF, &scancode) != 0) {
            kbc_error = true;
            return;
        }
        // Check for parity or timeout errors
        if ((status & KBC_PARITY_ERR) || (status & KBC_TIMEOUT_ERR)) {
            kbc_error = true;
        } else {
            scancode_ready = true;
        }
    }
}

// Helper function to write a command or argument to the KBC
int kbc_write_cmd(int port, uint8_t cmd) {
    uint8_t status;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return 1;
        // If Input Buffer is empty, we can write
        if ((status & KBC_IBF) == 0) {
            if (sys_outb(port, cmd) != 0) return 1;
            return 0; // Success
        }
        // Wait before trying again
        tickdelay(micros_to_ticks(DELAY_US));
    }
    return 1; // Timeout
}

// Helper function to read the command byte or a scancode directly with polling
int kbc_read_out_buf(uint8_t *data) {
    uint8_t status;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return 1;
        // If Output Buffer is full, we can read
        if (status & KBC_OBF) {
            if (util_sys_inb(KBC_OUT_BUF, data) != 0) return 1;
            // Check for communication errors
            if ((status & KBC_PARITY_ERR) || (status & KBC_TIMEOUT_ERR)) {
                return 1; 
            }
            return 0; // Success
        }
        // Wait before trying again
        tickdelay(micros_to_ticks(DELAY_US));
    }
    return 1; // Timeout
}

uint8_t get_scancode() {
    return scancode;
}

bool get_kbc_error() {
    return kbc_error;
}

bool get_scancode_ready() {
    return scancode_ready;
}
