// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "mouse.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
    int ipc_status, r;
    message msg;
    uint8_t mouse_bit_no = 2; // IRQ 12
    uint32_t irq_set_mouse = BIT(mouse_bit_no);
    int mouse_hook_id = 2;
    uint8_t mouse_bytes[3];
    uint8_t byte_index = 0;
    uint32_t packets_read = 0;
    struct packet pp;
    // Enable Mouse Data Reporting
    if (mouse_enable_data_reporting() != 0) return 1;
    // Subscribe to Interrupts (IRQ 12, Exclusive)
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != 0) return 1;
    while (packets_read < cnt) {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_mouse) {
                        mouse_ih(); // reads only one byte
                        if (get_mouse_error()) continue;
                        uint8_t current_byte = get_mouse_byte();
                        // Synchronization Check
                        if (byte_index == 0 && (current_byte & MOUSE_SYNC_BIT) == 0) {
                            continue; // it's out of sync, ignore 
                        }
                        // Store the byte in the array
                        mouse_bytes[byte_index] = current_byte;
                        byte_index++;
                        // When the packet is complete (3 bytes)
                        if (byte_index == 3) {
                            mouse_parse_packet(mouse_bytes, &pp);
                            mouse_print_packet(&pp);
                            byte_index = 0; // reset for the next packet
                            packets_read++; // increment the read packets
                        }
                    }
                    break;
                default: break;
            }
        }
    }
    // Clean up the subscriptions
    if (sys_irqrmpolicy(&mouse_hook_id) != 0) return 1;
    // Disable Data Reporting 
    if (mouse_write_cmd(MOUSE_DISABLE_DATA) != 0) return 1;
    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
    /* To be completed */
    printf("%s(%u): under construction\n", __func__, idle_time);
    return 1;
}

