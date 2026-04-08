#include <lcom/lcf.h>
#include "kbc.h"

// Global variables are used to share data between IH and the main function
static uint8_t scancode = 0; 
static bool kbc_error = false;

void (kbc_ih)() {
    uint8_t status;
    kbc_error = false; // reset error flag for each interrupt
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
        }
    }
}

uint8_t get_scancode() {
    return scancode;
}

bool get_kbc_error() {
    return kbc_error;
}
