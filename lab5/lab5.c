// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>
#include "video.h"
#include "vbe.h"
#include "../lab3/kbc.h"

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/lab5/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
        return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
    // Switch the video adapter to the specified graphics mode
    if (vbe_set_mode(mode) != 0) {
        printf("Error setting video mode!\n");
        return 1;
    }

    // Wait for the specified delay
    sleep(delay);

    // Return to MINIX's default text mode
    if (vg_exit() != 0) {
        printf("Error returning to text mode!\n");
        return 1;
    }

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
    // Map Video Memory
    if (vbe_map_vram(mode) != 0) {
        printf("Failed to map VRAM\n");
        return 1;
    }

    // Set Video Mode
    if (vbe_set_mode(mode) != 0) {
        printf("Failed to set video mode\n");
        return 1;
    }

    // Draw the rectangle
    vg_draw_rectangle(x, y, width, height, color);

    // Wait for ESC key (0x81) using Keyboard Interrupts
    int kbc_hook_id = KBC_IRQ; 
    uint32_t irq_set = BIT(kbc_hook_id);
    
    // Subscribe Keyboard Interrupts
    if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != 0) return 1;

    int ipc_status, r;
    message msg;
    bool esc_pressed = false;

    while (!esc_pressed) {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) {
                        kbc_ih(); 
                        if (get_kbc_error()) continue; 
                        if (get_scancode_ready() && get_scancode() == ESC_BREAK) { 
                            esc_pressed = true;
                        }
                    }
                    break;
            }
        }
    }

    // Clean up and Exit
    if (sys_irqrmpolicy(&kbc_hook_id) != 0) return 1; // unsubscribe
    if (vg_exit() != 0) return 1; // return to text mode

    return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
    uint16_t mode = 0x105;

    // Map Video Memory 
    if (vbe_map_vram(mode) != 0) {
        printf("Failed to map VRAM\n");
        return 1;
    }

    // Set Video Mode (0x105 for indexed colors)
    if (vbe_set_mode(mode) != 0) {
        printf("Failed to set video mode\n");
        return 1;
    }

    // Load the XPM into a Pixmap
    xpm_image_t img;
    // We use XPM_INDEXED because mode 0x105 uses 1 byte per color
    uint8_t *map = xpm_load(xpm, XPM_INDEXED, &img);
    if (map == NULL) {
        vg_exit();
        printf("Failed to load XPM\n");
        return 1;
    }

    // Draw the Pixmap pixel by pixel
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // Read the color from the 1D map array
            uint32_t color = map[i * img.width + j];
            // Draw it on the screen at the correct coordinates
            vg_draw_pixel(x + j, y + i, color);
        }
    }

    // Wait for ESC key (0x81) using Keyboard Interrupts
    int kbc_hook_id = KBC_IRQ; 
    uint32_t irq_set = BIT(kbc_hook_id);
    
    if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != 0) return 1;

    int ipc_status, r;
    message msg;
    bool esc_pressed = false;

    while (!esc_pressed) {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) {
                        kbc_ih(); 
                        if (get_kbc_error()) continue; 
                        
                        if (get_scancode_ready() && get_scancode() == ESC_BREAK) { 
                            esc_pressed = true;
                        }
                    }
                    break;
            }
        }
    }

    // Clean up and Exit
    if (sys_irqrmpolicy(&kbc_hook_id) != 0) return 1; 
    if (vg_exit() != 0) return 1;                     

    return 0;
}

