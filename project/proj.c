#include <lcom/lcf.h>
#include <lcom/timer.h>

#include "../lab3/kbc.h"
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
        printf("[TIMER] Erro: nao foi possivel configurar 60 Hz\n");
        return 1;
    }

    if (timer_subscribe_int(&timer_irq_set) != 0) {
        printf("[TIMER] Erro: nao foi possivel subscrever IRQ do timer\n");
        return 1;
    }

    printf("[TIMER] Timer 0 configurado a 60 Hz (irq_set = 0x%02X)\n",
           timer_irq_set);
    return 0;
}

static void timer_cleanup(void) {
    if (timer_unsubscribe_int() != 0)
        printf("[TIMER] Aviso: erro ao cancelar subscricao do timer\n");
}

int (proj_main_loop)(int argc, char *argv[]) {
    (void)argc; (void)argv;   /* Avoid unused parameter warnings. */

    if (timer_init_60hz() != 0)
        return 1;

    //ball_init(&g_ball, 0);

    if (vbe_map_vram(VIDEO_MODE) != 0) return 1;   
    if (vbe_set_mode(VIDEO_MODE) != 0) return 1;
    if (video_init_back_buffer() != 0) return 1;

    if (load_all_assets() != 0) {
        vg_exit(); 
        return 1;
    }

    game_init();

    if (kbd_subscribe_int(&kbd_irq_set) != 0) return 1;

    if (mouse_subscribe_int(&mouse_irq_set) != 0) {
        kbd_unsubscribe_int();
        timer_cleanup();
        video_free_back_buffer();
        vg_exit();
        return 1;
    }

    int running  = 1;
    int ipc_status;
    message msg;

    while (running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("[LOOP] Erro em driver_receive\n");
            continue;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
                        tick_count++;
                        timer_int_handler();

                        if (game_update() != 0 || game_should_exit()) {
                            running = 0;
                        } else {
                            game_render();
                        }

                        keyboard_clear_events();
                    }

                    if (msg.m_notify.interrupts & BIT(kbd_irq_set)) {
                        kbc_ih();

                        if (get_scancode_ready() && !get_kbc_error()) {
                            keyboard_handle(get_scancode());
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    }

    timer_cleanup();
    kbd_unsubscribe_int();
    
    video_free_back_buffer(); 
    vg_exit();

    printf("[EXIT] Ticks totais: %u  (~%.1f segundos)\n",
           tick_count, (float)tick_count / 60.0f);
    return 0;
}

int main(int argc, char *argv[]) {
    lcf_start(argc, argv);
    lcf_cleanup();
    return 0;
}
