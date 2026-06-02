#include "game.h"
#include "player.h"

#include "../devices/keyboard.h"
#include "../ui/game_render.h"
#include "../ui/menu.h"

/* Global game state */
GameState current_state = STATE_MENU;

/* Internal game objects */
static Ball g_ball;
static Player g_player1;
static Player g_player2;
static int score_p1 = 0;
static int score_p2 = 0;
static bool should_exit = false;

void game_init(void) {
    current_state = STATE_MENU;

    score_p1 = 0;
    score_p2 = 0;
    should_exit = false;

    g_ball.x = 512.0f;
    g_ball.y = 200.0f;
    g_ball.vx = 0.0f;
    g_ball.vy = 0.0f;

    player_init(&g_player1, 220.0f);
    player_init(&g_player2, 800.0f);
}

int game_update(void) {
    const KeyboardState *kb = get_keyboard_state();

    if (current_state == STATE_MENU) {
        if (kb->enter) {
            current_state = STATE_PLAYING;
        }

        if (kb->pause) {
            should_exit = true;
            return 1;
        }
    }

    else if (current_state == STATE_PLAYING) {
        if (kb->pause) {
            current_state = STATE_PAUSED;
            return 0;
        }

        player1_update(&g_player1);
        player2_update(&g_player2);

        /* TODO: move ball physics into its own module. */
        g_ball.vy += GRAVITY;
        g_ball.x += g_ball.vx;
        g_ball.y += g_ball.vy;

        if (g_ball.y > FLOOR_Y - 16) {
            g_ball.y = FLOOR_Y - 16;
            g_ball.vy = -8.0f;
        }

        /* TODO: replace this with left/right floor detection. */
        if (score_p1 >= WIN_SCORE || score_p2 >= WIN_SCORE) {
            current_state = STATE_END;
        }
    }

    else if (current_state == STATE_PAUSED) {
        if (kb->enter) {
            current_state = STATE_PLAYING;
        }

        if (kb->pause) {
            current_state = STATE_MENU;
        }
    }

    else if (current_state == STATE_END) {
        if (kb->enter) {
            game_init();
        }

        if (kb->pause) {
            should_exit = true;
            return 1;
        }
    }

    return 0;
}

void game_render(void) {
    if (current_state == STATE_MENU) {
        draw_main_menu();
    }

    else if (current_state == STATE_PLAYING) {
        draw_field();
        video_draw_net();
        video_draw_ball(&g_ball);
        video_draw_players(&g_player1, &g_player2);
    }

    else if (current_state == STATE_PAUSED) {
        draw_field();
        video_draw_net();
        video_draw_ball(&g_ball);
        video_draw_players(&g_player1, &g_player2);
        draw_pause_menu();
    }

    else if (current_state == STATE_END) {
        draw_main_menu();
    }

    video_flush();
}

bool game_should_exit(void) {
    return should_exit;
}
