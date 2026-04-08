#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include "kbc.h"

uint32_t get_sys_inb_cnt();

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
  bool make;
  uint8_t current_scancode = 0;
  // Subscribe to keyboard interrupts
  if (sys_irqsetpolicy(1, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != 0) {
    printf("There was an error while subscribing keyboard interruptions\n");
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
                  if (get_kbc_error()) continue; // if there was an error while reading, ignore
                  current_scancode = get_scancode();
                  if (current_scancode == TWO_BYTE_PREFIX) {
                    // First byte of a two byte scancode
                    bytes[0] = current_scancode;
                    size = 2;
                  } else {
                    if (size == 2) {
                      // Second byte of a two byte scancode
                      bytes[1] = current_scancode;
                    } else {
                      // It's a one byte scancode
                      bytes[0] = current_scancode;
                    }
                    /* Check MSB to determine if it's a Makecode or a Breakcode. If bit 7 is 0, it's a makecode. Else it's a breakcode. */
                    make = !(current_scancode & BIT(7));
                    kbd_print_scancode(make, size, bytes);
                    size = 1; // reset the size for the next scancode
                  }
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
  uint32_t cnt = get_sys_inb_cnt();
  kbd_print_no_sysinb(cnt);
  return 0;
}

int(kbd_test_poll)() {
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int(kbd_test_timed_scan)(uint8_t n) {
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

