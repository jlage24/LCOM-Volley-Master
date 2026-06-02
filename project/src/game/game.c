#include "game.h"
#include "player.h"
#include "ball.h"

#include "../devices/keyboard.h"
#include "../devices/mouse.h"
#include "../ui/game_render.h"
#include "../ui/menu.h"

#define MENU_BUTTON_X 80
#define MENU_START_Y 380
#define MENU_EXIT_Y 460
#define MENU_BUTTON_W 300
#define MENU_BUTTON_H 130

/* Global game state */
GameState current_state = STATE_MENU;

/* Internal game objects */
static Ball g_ball;
static Player g_player1;
static Player g_player2;
static int score_p1 = 0;
static int score_p2 = 0;
static int cursor_x = H_RES / 2;
static int cursor_y = V_RES / 2;
static bool mouse_button_was_down = false;
static bool should_exit = false;

static bool point_inside_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

static void update_menu_cursor(void) {
    cursor_x += mouse_state.dx;
    cursor_y -= mouse_state.dy;

    if (cursor_x < 0) cursor_x = 0;
    if (cursor_x > H_RES - 1) cursor_x = H_RES - 1;
    if (cursor_y < 0) cursor_y = 0;
    if (cursor_y > V_RES - 1) cursor_y = V_RES - 1;
}

static int finish_update(int status) {
    mouse_button_was_down = mouse_state.left_button;
    return status;
}

void game_init(void) {
    current_state = STATE_MENU;

    score_p1 = 0;
    score_p2 = 0;
    cursor_x = H_RES / 2;
    cursor_y = V_RES / 2;
    mouse_button_was_down = false;
    should_exit = false;

    ball_init(&g_ball, 1);

    player_init(&g_player1, 220.0f);
    player_init(&g_player2, 800.0f);
}

int game_update(void) {
    const KeyboardState *kb = get_keyboard_state();
    bool mouse_clicked = mouse_state.left_button && !mouse_button_was_down;

    update_menu_cursor();

    if (current_state == STATE_MENU) {
        if (kb->enter) {
            current_state = STATE_PLAYING;
        }

        if (kb->pause) {
            should_exit = true;
            return finish_update(1);
        }

        if (current_state == STATE_MENU && mouse_clicked) {
            if (point_inside_rect(cursor_x, cursor_y,
                                  MENU_BUTTON_X, MENU_EXIT_Y,
                                  MENU_BUTTON_W, MENU_BUTTON_H)) {
                should_exit = true;
                return finish_update(1);
            }
            else if (point_inside_rect(cursor_x, cursor_y,
                                       MENU_BUTTON_X, MENU_START_Y,
                                       MENU_BUTTON_W, MENU_BUTTON_H)) {
                current_state = STATE_PLAYING;
            }
        }
    }

    else if (current_state == STATE_PLAYING) {
        if (kb->pause) {
            current_state = STATE_PAUSED;
            return finish_update(0);
        }

        player1_update(&g_player1);
        player2_update(&g_player2);

        int point = ball_update(&g_ball);

        ball_collide_with_player(
            &g_ball,
            &g_player1,
            +1
        );

        ball_collide_with_player(
            &g_ball,
            &g_player2,
            -1
        );

        if (point != 0) {
            if (point == 1) {
                score_p1++;
                ball_init(&g_ball, -1);
            } else if (point == -1) {
                score_p2++;
                ball_init(&g_ball, 1);
            }
            player_init(&g_player1, 220.0f);
            player_init(&g_player2, 800.0f);
        }
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

        if (current_state == STATE_PAUSED && mouse_clicked) {
            if (point_inside_rect(cursor_x, cursor_y,
                                  MENU_BUTTON_X, MENU_EXIT_Y,
                                  MENU_BUTTON_W, MENU_BUTTON_H)) {
                current_state = STATE_MENU;
            }
            else if (point_inside_rect(cursor_x, cursor_y,
                                       MENU_BUTTON_X, MENU_START_Y,
                                       MENU_BUTTON_W, MENU_BUTTON_H)) {
                current_state = STATE_PLAYING;
            }
        }
    }

    else if (current_state == STATE_END) {
        if (kb->enter) {
            game_init();
        }

        if (kb->pause) {
            should_exit = true;
            return finish_update(1);
        }
    }

    return finish_update(0);
}

void game_render(void) {
    if (current_state == STATE_MENU) {
        draw_main_menu();
        video_draw_cursor(cursor_x, cursor_y);
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
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_END) {
        draw_main_menu();
    }

    video_flush();
}

bool game_should_exit(void) {
    return should_exit;
}
