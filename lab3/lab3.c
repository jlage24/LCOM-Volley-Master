#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include "kbc.h"

uint32_t get_sys_inb_cnt();
uint32_t get_timer_ticks();
void reset_timer_ticks();

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

void process_scancode(uint8_t current_scancode, uint8_t *bytes, uint8_t *size) {
  if (current_scancode == TWO_BYTE_PREFIX) {
    // First byte of a two byte scancode
    bytes[0] = current_scancode;
    *size = 2;
  } else {
    if (*size == 2) {
      // Second byte of a two byte scancode
      bytes[1] = current_scancode;
    } else {
      // It's a one byte scancode
      bytes[0] = current_scancode;
    }
    /* Check MSB to determine if it's a Makecode or a Breakcode. If bit 7 is 0, it's a makecode. Else it's a breakcode. */
    bool make = !(current_scancode & BIT(7));
    kbd_print_scancode(make, *size, bytes);
    *size = 1; // reset the size for the next scancode
  }
}

int(kbd_test_scan)() {
  int ipc_status, r;
  message msg;
  // Variables for subscription
  uint8_t bit_no = 1; // keyboard's IRQ is 1
  uint32_t irq_set = BIT(bit_no);
  int kbc_hook_id = 1;
  // Variables for scancode's processing
  uint8_t bytes[2];
  uint8_t size = 1;
  uint8_t current_scancode = 0;
  // Subscribe to keyboard interrupts
  if (sys_irqsetpolicy(1, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != 0) {
    printf("There was an error while subscribing keyboard interrupts \n");
    return 1;
  }
  // The loop will continue until the scancode is the Breakcode for ESC
  while (current_scancode != ESC_BREAK) {
    /* Get a request message. */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */
                if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                  kbc_ih(); // call Interrupt Handler
                  if (get_kbc_error() || !get_scancode_ready()) continue; // if there was an error while reading, ignore
                  current_scancode = get_scancode();
                  process_scancode(current_scancode, bytes, &size);
                }
                break;
            default:
                break; /* no other notifications expected: do nothing */
        }
    } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
    }
  }
  // Cancel interrupt subscription before leaving, so that the keyboard continues working in Minix
  if (sys_irqrmpolicy(&kbc_hook_id) != 0) {
    printf("Error while canceling keyboard's subscription\n");
    return 1;
  }
  kbd_print_no_sysinb(get_sys_inb_cnt());
  return 0;
}

int(kbd_test_poll)() {
  uint8_t cmd_byte;
  uint8_t current_scancode = 0;
  uint8_t bytes[2];
  uint8_t size = 1;
  while (current_scancode != ESC_BREAK) {
    // Read the status directly
    uint8_t status;
    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) continue;
    // If there's data, let's get it and validate errors
    if (status & KBC_OBF) {
      if (util_sys_inb(KBC_OUT_BUF, &current_scancode) != 0) continue;
      if ((status & KBC_PARITY_ERR) || (status & KBC_TIMEOUT_ERR)) continue;
      process_scancode(current_scancode, bytes, &size);
    } else {
      // If there's no data, do a pause
      tickdelay(micros_to_ticks(DELAY_US));
    }
  }
  /* Restore the KBC */
  // Read the Comand Byte status
  if (kbc_write_cmd(KBC_STATUS_REG, KBC_READ_CMD) != 0) return 1;
  if (kbc_read_out_buf(&cmd_byte) != 0) return 1;
  // Activate keyboard interrupts (setting Bit 0 to 1)
  cmd_byte = cmd_byte | KBC_INT_KBD;
  // Write the corrected configuration
  if (kbc_write_cmd(KBC_STATUS_REG, KBC_WRITE_CMD) != 0) return 1;
  if (kbc_write_cmd(KBC_OUT_BUF, cmd_byte) != 0) return 1;
  kbd_print_no_sysinb(get_sys_inb_cnt());
  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status, r;
  message msg;
  /* Keyboard variables */
  uint8_t kbd_bit_no = 1;
  uint32_t irq_set_kbd = BIT(kbd_bit_no);
  int kbc_hook_id = 1; 
  uint8_t bytes[2];
  uint8_t size = 1;
  uint8_t current_scancode = 0;
  /* Timer variables */
  uint8_t timer_bit_no = 0;
  // Subscribe to both Timer and Keyboard interrupts
  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != 0) {
    printf("Error subscribing keyboard interrupts\n");
    return 1;
  }
  if (timer_subscribe_int(&timer_bit_no) != 0) {
    printf("Error subscribing timer interrupts\n");
    return 1;
  }
  uint32_t irq_set_timer = BIT(timer_bit_no); //create the timer IRQ mask after subscribing
  reset_timer_ticks(); 
  // The timer frequency is 60Hz. So, n seconds = n * 60 ticks.
  uint32_t limit_ticks = n * 60;
  while (current_scancode != ESC_BREAK && get_timer_ticks() < limit_ticks) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { 
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: 
            /* Process Timer Interrupt */
            if (msg.m_notify.interrupts & irq_set_timer) {
                timer_int_handler(); // increments timer_ticks
            }
            /* Process Keyboard Interrupt */
            if (msg.m_notify.interrupts & irq_set_kbd) { 
                kbc_ih(); 
                if (get_kbc_error() || !get_scancode_ready()) continue; 
                current_scancode = get_scancode();
                process_scancode(current_scancode, bytes, &size);
                // Reset the timer when user activity is detected
                reset_timer_ticks();
            }
            break;
        default:
            break; 
      }
    }
  }
  // Unsubscribe from all devices before leaving
  if (sys_irqrmpolicy(&kbc_hook_id) != 0) {
      printf("Error unsubscribing keyboard\n");
      return 1;
  }
  if (timer_unsubscribe_int() != 0) {
      printf("Error unsubscribing timer\n");
      return 1;
  }
  kbd_print_no_sysinb(get_sys_inb_cnt());
  return 0;
}
