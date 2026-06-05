/*
 * LCOM project entry point and interrupt loop.
 *
 * This file configures the timer, video mode, keyboard, mouse and assets, then
 * drives the game through hardware notifications. The game module receives one
 * update/render opportunity per 60 Hz timer tick; device interrupts only update
 * input state consumed on the next tick.
 */
#include <lcom/lcf.h>
#include <lcom/timer.h>

#include "../lab3/kbc.h"
#include "../lab4/mouse.h"
#include "../lab5/video.h"

#include "game/game.h"

#include "devices/keyboard.h"
#include "devices/mouse.h"
#include "devices/rtc.h"

#include "ui/game_render.h"
#include "ui/menu.h"
#include "ui/assets.h"

static uint8_t timer_irq_set = 0;
static uint8_t kbd_irq_set = 0;
static uint8_t mouse_irq_set = 0;

/* Used for the exit log. */
static uint32_t tick_count = 0;

static int timer_init_60hz(void) {
    if (timer_set_frequency(0, 60) != 0) {
        printf("[TIMER] Error: could not configure 60 Hz\n");
        return 1;
    }

    if (timer_subscribe_int(&timer_irq_set) != 0) {
        printf("[TIMER] Error: could not subscribe timer IRQ\n");
        return 1;
    }

    printf("[TIMER] Timer 0 configured to 60 Hz (irq_set = 0x%02X)\n",
           timer_irq_set);
    return 0;
}

static void timer_cleanup(void) {
    if (timer_unsubscribe_int() != 0)
        printf("[TIMER] Warning: error while unsubscribing timer\n");
}

int (proj_main_loop)(int argc, char *argv[]) {
    (void)argc; (void)argv;   /* Avoid unused parameter warnings. */

    /* The timer is the heartbeat for both gameplay time and animation. */
    if (timer_init_60hz() != 0)
        return 1;

    /* Video and assets are initialized before game state so rendering is ready. */
    if (vbe_map_vram(VIDEO_MODE) != 0) return 1;   
    if (vbe_set_mode(VIDEO_MODE) != 0) return 1;
    if (video_init_back_buffer() != 0) return 1;

    if (load_all_assets() != 0) {
        free_all_assets();
        video_free_back_buffer();
        vg_exit(); 
        return 1;
    }

    game_init();

    if (kbd_subscribe_int(&kbd_irq_set) != 0) {
        timer_cleanup();
        free_all_assets();
        video_free_back_buffer();
        vg_exit();
        return 1;
    }

    if (mouse_subscribe_int(&mouse_irq_set) != 0) {
        kbd_unsubscribe_int();
        timer_cleanup();
        free_all_assets();
        video_free_back_buffer();
        vg_exit();
        return 1;
    }

    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) {
        printf("[MOUSE] Error: could not enable data reporting\n");
        mouse_unsubscribe_int();
        kbd_unsubscribe_int();
        timer_cleanup();
        free_all_assets();
        video_free_back_buffer();
        vg_exit();
        return 1;
    }

    int running  = 1;
    int ipc_status;
    message msg;

    while (running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("[LOOP] Error in driver_receive\n");
            continue;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
                        /*
                         * Game logic advances only on timer ticks. This keeps
                         * physics deterministic and prevents keyboard/mouse IRQ
                         * rate from changing gameplay speed.
                         */
                        tick_count++;
                        timer_int_handler();

                        if (game_update() != 0 || game_should_exit()) {
                            running = 0;
                        } else {
                            game_render();
                        }

                        keyboard_clear_events();
                        mouse_clear_state();
                    }

                    if (msg.m_notify.interrupts & BIT(kbd_irq_set)) {
                        /* Keyboard IRQs translate raw scancodes into game input state. */
                        kbc_ih();

                        if (get_scancode_ready() && !get_kbc_error()) {
                            keyboard_handle(get_scancode());
                        }
                    }

                    if (msg.m_notify.interrupts & BIT(mouse_irq_set)) {
                        /* Mouse IRQs collect bytes until a complete packet is available. */
                        mouse_ih();

                        if (!get_mouse_error()) {
                            mouse_handle_bytes(get_mouse_byte());
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    }

    /* Cleanup mirrors initialization order so devices stop producing input first. */
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    mouse_unsubscribe_int();

    kbd_unsubscribe_int();
    timer_cleanup();

    free_all_assets();
    video_free_back_buffer();
    vg_exit();

    printf("[EXIT] Total ticks: %u  (~%.1f seconds)\n",
           tick_count, (float)tick_count / 60.0f);
    return 0;
}

int main(int argc, char *argv[]) {
    lcf_start(argc, argv);
    lcf_cleanup();
    return 0;
}
