#include <lcom/lcf.h>
#include "mouse.h"
#include "kbc.h"

static uint8_t current_byte;
static bool mouse_error = false;

void (mouse_ih)() {
    uint8_t status;
    mouse_error = false; // reset error flag for each interrupt
    // Read the status register
    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) { 
      mouse_error = true; 
      return; 
    }
    // Check if OBF is full (data is available)
    if (status & KBC_OBF) {
        // Read the Output Buffer
        if (util_sys_inb(KBC_OUT_BUF, &current_byte) != 0) { 
          mouse_error = true; 
          return; 
        }
        // Check for parity or timeout errors
        if ((status & KBC_PARITY_ERR) || (status & KBC_TIMEOUT_ERR)) {
            mouse_error = true;
        }
    }
}

uint8_t get_mouse_byte() { return current_byte; }
bool get_mouse_error() { return mouse_error; }

int mouse_write_cmd(uint8_t cmd) {
    uint8_t ack;
    for (int i = 0; i < MAX_RETRIES; i++) {
        // Tell the KBC we are going to send a byte to the mouse
        if (kbc_write_cmd(KBC_STATUS_REG, KBC_WRITE_TO_MOUSE) != 0) return 1;
        // Send the actual mouse command (e.g., 0xF5)
        if (kbc_write_cmd(KBC_OUT_BUF, cmd) != 0) return 1;
        // Read the Acknowledgement from the Output Buffer
        if (kbc_read_out_buf(&ack) != 0) return 1;
        // Check the response
        if (ack == MOUSE_ACK) return 0;     // success
        if (ack == MOUSE_ERROR) return 1;   // fatal error
        // If it's MOUSE_NACK (0xFE), the for loop repeats the attempt
    }
    return 1;
}

// Builds the struct with the 3 bytes based on Table 2 of the lab4 handout
void mouse_parse_packet(uint8_t *bytes, struct packet *pp) {
  // Store the raw packet bytes in the struct's array
  pp->bytes[0] = bytes[0];
  pp->bytes[1] = bytes[1];
  pp->bytes[2] = bytes[2];
  /* 
  Extract boolean flags from Byte 0 using bitwise AND (&) operations
  If the specific bit is 1, the result is non-zero (true). Otherwise, it's 0 (false).
  */
  pp->lb = bytes[0] & MOUSE_LB; // Check if Left Button is currently pressed
  pp->rb = bytes[0] & MOUSE_RB; // Check if Right Button is currently pressed
  pp->mb = bytes[0] & MOUSE_MB; // Check if Middle Button is currently pressed
  // Extract overflow flags from Byte 0
  pp->x_ov = bytes[0] & MOUSE_X_OVF; // Check if X movement exceeded the 8-bit limit
  pp->y_ov = bytes[0] & MOUSE_Y_OVF; // Check if Y movement exceeded the 8-bit limit
  
  // Sign extension to 16 bits (because distances can be negative)
  if (bytes[0] & MOUSE_X_SIGN) {
      pp->delta_x = 0xFF00 | bytes[1];
  } else {
      pp->delta_x = bytes[1];
  }
  
  if (bytes[0] & MOUSE_Y_SIGN) {
      pp->delta_y = 0xFF00 | bytes[2];
  } else {
      pp->delta_y = bytes[2];
  }
}


MouseState mouse_state = {0, false, false};

static uint8_t mouse_buf[3];
static uint8_t mouse_idx = 0;

void mouse_handle_bytes(uint8_t byte) {

    if (get_mouse_error()){
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
        if (pp.x_ov) {
            mouse_state.dx = 0;
        } else {
            mouse_state.dx = pp.delta_x;
        }

        mouse_state.jump = pp.lb;
        mouse_state.new_data = true;

        mouse_idx = 0;
        
    }
}