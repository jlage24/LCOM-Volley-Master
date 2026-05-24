/*

 * Estrutura do ciclo de eventos (Minix IPC):
 *
 *   ┌─────────────────────────────────┐
 *   │  driver_receive(&msg)           │  ← bloqueia até haver interrupção
 *   │                                 │
 *   │  se msg = TIMER_IRQ             │
 *   │      tick_count++               │
 *   │      game_update()              │  ← física + lógica (60 Hz fixo)
 *   │      video_draw_frame()         │  ← render (da Mafalda)
 *   │                                 │
 *   │  se msg = KBD_IRQ / MOUSE_IRQ   │  ← João / David tratam aqui
 *   │      ...                        │
 *   └─────────────────────────────────┘
 *
 * Frequência : O timer 0 do PC é configurado a 60 Hz pela libtimer.a.
 *   timer_set_frequency(0, 60)  →  IRQ 0  →  BIT(0) no sys_irqsetpolicy
 */

#include <lcom/lcf.h>      /* driver_receive, message, etc.  */
#include <lcom/timer.h>
#include "../lab3/kbc.h"
#include "../lab5/video.h"

/* Cabeçalhos do projeto 
#include "game/game.h"
#include "game/ball.h" 
*/
/* #include "game/player.h"  ← descomentar quando o João entregar */

#include "src/input/keyboard.h"
#include "ui/game_render.h"
#include "ui/menu.h"
#include "ui/assets.h"

GameState current_state = STATE_PLAYING;

/* Variáveis globais do módulo Timer*/

/* Bit de interrupção (preenchido pelo subscribe) */
static uint8_t timer_irq_set = 0;
static uint8_t kbd_irq_set = 0;

/* Contador de ticks – útil para debug e animações */
static uint32_t tick_count = 0;

/* ─────────────────────────────────────────────
 * Estado global do jogo (pode migrar para game.c)
 * ───────────────────────────────────────────── */
static Ball  g_ball;
static int   score_p1 = 0;
static int   score_p2 = 0;


/* Funções auxiliares do Timer
 * ═══════════════════════════════════════════════════════ */

/**
 * timer_init_60hz
 *
 * Configura o Timer 0 do PC a 60 Hz e subscreve as interrupções.
 * Deve ser chamada UMA vez antes do game loop.
 *
 * @return 0 em sucesso, valor diferente em erro.
 */
static int timer_init_60hz(void) {
    /*
     * 1. Fixa a frequência do timer 0 a 60 Hz.
     *    (A frequência base do PC é ~1193182 Hz; a libtimer
     *     calcula o divisor automaticamente.)
     */
    if (timer_set_frequency(0, 60) != 0) {
        printf("[TIMER] Erro: nao foi possivel configurar 60 Hz\n");
        return 1;
    }

    /*
     * 2. Subscreve as interrupções do timer.
     *    timer_subscribe_int() preenche timer_irq_set com o bit
     *    correspondente ao IRQ 0. Esse bit é usado no loop para
     *    distinguir qual interrupção chegou.
     */
    if (timer_subscribe_int(&timer_irq_set) != 0) {
        printf("[TIMER] Erro: nao foi possivel subscrever IRQ do timer\n");
        return 1;
    }

    printf("[TIMER] Timer 0 configurado a 60 Hz (irq_set = 0x%02X)\n",
           timer_irq_set);
    return 0;
}

/**
 * timer_cleanup
 *
 * Cancela a subscrição das interrupções do timer.
 * Deve ser chamada ao sair do jogo.
 */
static void timer_cleanup(void) {
    if (timer_unsubscribe_int() != 0)
        printf("[TIMER] Aviso: erro ao cancelar subscricao do timer\n");
}

/* ═══════════════════════════════════════════════════════
 * game_update  (chamada a cada tick = cada 1/60 s)
 * ═══════════════════════════════════════════════════════ */

/**
 * game_update
 *
 * Avança um frame da lógica do jogo:
 *   1. Física da bola.
 *   2. Deteção de ponto + reset.
 *   3. Colisões bola↔jogadores.  (Semanas 3/4 – Torrão)
 *   4. Lógica de vitória.
 *
 * @return 0 → continua o jogo
 *         1 → alguém ganhou, sair do loop
 */
int game_update(void) {

    const KeyboardState *kb = get_keyboard_state();

    if (current_state == STATE_MENU) {
        if (kb->enter) {
            current_state = STATE_PLAYING; 
        }
        if (kb->pause) {
            return 1; 
        }
    } 
    else if (current_state == STATE_PLAYING) {
        if (kb->pause) {
            current_state = STATE_PAUSED;
        }
        /* ── Física da bola ───────────────────────────────── */

        int point = 0;

        /* ── Ponto marcado? ───────────────────────────────── */

        if (point == 1) {
            score_p1++;
            printf("[SCORE] Jogador 1: %d  |  Jogador 2: %d\n",
                score_p1, score_p2);
            //ball_init(&g_ball, 1);   /* serve para a direita */
        } else if (point == -1) {
            score_p2++;
            printf("[SCORE] Jogador 1: %d  |  Jogador 2: %d\n",
                score_p1, score_p2);
            //ball_init(&g_ball, 0);   /* serve para a esquerda */
        }

        /* ── Verificação de vitória ───────────────────────── */

        if (score_p1 >= WIN_SCORE) {
            printf("[FIM]  Jogador 1 ganhou!\n");
            return 1;
        }
        if (score_p2 >= WIN_SCORE) {
            printf("[FIM]  Jogador 2 ganhou!\n");
            return 1;
        }
    } 
    else if (current_state == STATE_PAUSED) {
        if (kb->enter) {
            current_state = STATE_PLAYING; // O ENTER tira a pausa
        }
        if (kb->pause) {
            current_state = STATE_MENU; // O ESC desiste e volta ao Menu
        }
    }
    
    /*
     * ── Colisões bola↔jogadores  (SEMANAS 3/4) ─────────
     *
     * TODO (Torrão): descomentar quando player.h estiver disponível.
     *
     * extern Player g_player1, g_player2;
     *
     * if (ball_collides_player(&g_ball,
     *         g_player1.x, g_player1.y, PLAYER_RADIUS)) {
     *     ball_apply_player_impulse(&g_ball,
     *         g_player1.x, g_player1.y,
     *         g_player1.vx, g_player1.vy,
     *         PLAYER_RADIUS);
     * }
     * if (ball_collides_player(&g_ball,
     *         g_player2.x, g_player2.y, PLAYER_RADIUS)) {
     *     ball_apply_player_impulse(&g_ball,
     *         g_player2.x, g_player2.y,
     *         g_player2.vx, g_player2.vy,
     *         PLAYER_RADIUS);
     * }
     */

    /*
     * ── Render  (Mafalda) ───────────────────────────────
     */
    if (current_state == STATE_MENU) {
        draw_main_menu();
    }
    else if (current_state == STATE_PLAYING) {
        video_draw_background();
        video_draw_net();
        video_draw_ball(&g_ball);
    }
    else if (current_state == STATE_PAUSED) {
        video_draw_background();
        video_draw_net();
        video_draw_ball(&g_ball);
        // Desenha o menu de pausa por cima
        draw_pause_menu();
    }
    else if (current_state == STATE_END) {
        draw_game_over_menu();
    }
    

    // video_draw_players(&g_player1, &g_player2);

    video_flush();

    return 0;
}

/* ═══════════════════════════════════════════════════════
 * proj_main_loop  (chamada pelo LCF após inicialização)
 * ═══════════════════════════════════════════════════════ */
int (proj_main_loop)(int argc, char *argv[]) {
    (void)argc; (void)argv;   /* evita warnings de unused */

    /* ── Inicialização ────────────────────────────────── */

    /* Timer */
    if (timer_init_60hz() != 0)
        return 1;

    /* Estado inicial da bola (serve para a esquerda no arranque) */
    //ball_init(&g_ball, 0);
    g_ball.x = 512.0f; 
    g_ball.y = 200.0f; 
   

    /*
     * TO DO (Mafalda): inicializar o modo de vídeo aqui. */

    if (vbe_map_vram(0x118) != 0) return 1;    // FORÇAR 0x118 AQUI
    if (vbe_set_mode(0x118) != 0) return 1;
    if (video_init_back_buffer() != 0) return 1;

    if (load_all_assets() != 0) {
        vg_exit(); 
        return 1;
    }

    /* TO DO (João/David): subscrever teclado e rato aqui. 
     mouse_subscribe_int(&mouse_irq_set);
     */

    if (kbd_subscribe_int(&kbd_irq_set) != 0) return 1;

    printf("[INIT] Volley Master iniciado. Bola em (%.0f, %.0f)\n",
           g_ball.x, g_ball.y);

    /* ── Game Loop ────────────────────────────────────── */
    int running  = 1;
    int ipc_status;
    message msg;

    while (running) {
        /* Aguarda próxima mensagem de interrupção (bloqueia aqui) */
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("[LOOP] Erro em driver_receive\n");
            continue;
        }

        /* Filtra notificações vs mensagens normais */
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {

                case HARDWARE:
                    /* ── Interrupção do Timer (60 Hz) ─── */
                    if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
                        tick_count++;
                        timer_int_handler();   /* limpa o contador interno da libtimer */

                        if (game_update() != 0)
                            running = 0;       /* alguém ganhou → sair   */
                            
                        keyboard_clear_events();
                    }

                    if (msg.m_notify.interrupts & BIT(kbd_irq_set)) {
                        kbc_ih(); // Lê a tecla
                        if (get_scancode_ready() && !get_kbc_error()) {
                            keyboard_handle(get_scancode()); 
                        }
                    }

                    /*
                     * ── Teclado / Rato (João / David) ──
                     *
                     * TODO: descomentar quando kbd/mouse estiverem prontos.
                     *
                     * if (msg.m_notify.interrupts & kbd_irq_set) {
                     *     kbd_int_handler();
                     *     if (kbd_get_scancode() == ESC_SCANCODE)
                     *         running = 0;
                     * }
                     * if (msg.m_notify.interrupts & mouse_irq_set) {
                     *     mouse_int_handler();
                     * }
                     */
                    break;

                default:
                    break;
            }
        }
    }

    /* ── Limpeza ──────────────────────────────────────── */
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

