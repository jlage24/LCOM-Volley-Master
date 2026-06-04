#include "game.h"
#include "player.h"
#include "ball.h"

#include "../devices/keyboard.h"
#include "../devices/mouse.h"
#include "../ui/assets.h"
#include "../ui/game_render.h"
#include "../ui/menu.h"

#define MENU_BUTTON_X 80
#define MENU_START_Y 380
#define MENU_EXIT_Y 460
#define MENU_BUTTON_W 300
#define MENU_BUTTON_H 130
#define FIELD_CARD_Y 388
#define FIELD_CARD_W 190
#define FIELD_CARD_H 205

/* Global game state */
GameState current_state = STATE_MENU;
t_field_type selected_field = FIELD_JUNGLE;

/* Internal game objects */
static Ball g_ball;
static Player g_player1;
static Player g_player2;
static int score_p1 = 0;
static int score_p2 = 0;
static int game_seconds_left = GAME_TIME_SECONDS;
static int sudden_death_seconds = 0;
static int timer_frame_count = 0;
static int cursor_x = H_RES / 2;
static int cursor_y = V_RES / 2;
static bool mouse_button_was_down = false;
static bool should_exit = false;
static bool sudden_death = false;
static int selected_player1 = -1;
static int selected_player2 = -1;
static bool choosing_player2 = false;
static const int character_card_x[PLAYER_CHARACTER_COUNT] = {
    112, 247, 381, 514, 647, 780
};
static const int field_card_x[FIELD_COUNT] = {
    126, 323, 521, 720
};

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

static void reset_round(int ball_direction) {
    ball_init(&g_ball, ball_direction);

    player_init(&g_player1, 220.0f);
    player_init(&g_player2, 800.0f);
}

static void reset_match(void) {
    score_p1 = 0;
    score_p2 = 0;
    game_seconds_left = GAME_TIME_SECONDS;
    sudden_death_seconds = 0;
    timer_frame_count = 0;
    sudden_death = false;
    reset_round(1);
}

static int character_at_point(int x, int y) {
    for (int i = 0; i < PLAYER_CHARACTER_COUNT; i++) {
        int card_x = character_card_x[i];

        if (point_inside_rect(x, y, card_x, CHARACTER_CARD_Y,
                              CHARACTER_CARD_W, CHARACTER_CARD_H)) {
            return i;
        }
    }

    return -1;
}

static t_field_type field_at_point(int x, int y) {
    for (int i = 0; i < FIELD_COUNT; i++) {
        if (point_inside_rect(x, y, field_card_x[i], FIELD_CARD_Y,
                              FIELD_CARD_W, FIELD_CARD_H)) {
            return (t_field_type)i;
        }
    }

    return FIELD_COUNT;
}

static void open_character_select(void) {
    selected_player1 = -1;
    selected_player2 = -1;
    selected_field = FIELD_JUNGLE;
    choosing_player2 = false;
    current_state = STATE_CHARACTER_SELECT;
}

static int first_available_character(int unavailable_character) {
    for (int i = 0; i < PLAYER_CHARACTER_COUNT; i++) {
        if (i != unavailable_character) {
            return i;
        }
    }

    return 0;
}

static void start_match_from_selection(void) {
    if (selected_player1 < 0) selected_player1 = 0;
    if (selected_player2 < 0 || selected_player2 == selected_player1) {
        selected_player2 = first_available_character(selected_player1);
    }

    player1_pixmap = &player_pixmaps[selected_player1];
    player2_pixmap = &player_pixmaps[selected_player2];
    current_state = STATE_FIELD_SELECT;
}

static void start_match_from_field_selection(void) {
    reset_match();
    current_state = STATE_PLAYING;
}

static void check_win_after_point(void) {
    if (sudden_death) {
        current_state = STATE_END;
        return;
    }

    if (score_p1 >= WIN_SCORE || score_p2 >= WIN_SCORE) {
        if (score_p1 == score_p2) {
            sudden_death = true;
            sudden_death_seconds = 0;
            timer_frame_count = 0;
        } else {
            current_state = STATE_END;
        }
    }
}

static void update_game_timer(void) {
    timer_frame_count++;

    if (timer_frame_count < 60) {
        return;
    }

    timer_frame_count = 0;

    if (sudden_death) {
        sudden_death_seconds++;
        return;
    }

    if (game_seconds_left > 0) {
        game_seconds_left--;
    }

    if (game_seconds_left == 0) {
        if (score_p1 == score_p2) {
            sudden_death = true;
            sudden_death_seconds = 0;
            timer_frame_count = 0;
        } else {
            current_state = STATE_END;
        }
    }
}

void game_init(void) {
    current_state = STATE_MENU;

    score_p1 = 0;
    score_p2 = 0;
    game_seconds_left = GAME_TIME_SECONDS;
    sudden_death_seconds = 0;
    timer_frame_count = 0;
    cursor_x = H_RES / 2;
    cursor_y = V_RES / 2;
    mouse_button_was_down = false;
    should_exit = false;
    sudden_death = false;
    selected_player1 = -1;
    selected_player2 = -1;
    selected_field = FIELD_JUNGLE;
    choosing_player2 = false;
    player1_pixmap = &player_pixmaps[0];
    player2_pixmap = &player_pixmaps[1];

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
            open_character_select();
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
                open_character_select();
            }
        }
    }

    else if (current_state == STATE_CHARACTER_SELECT) {
        if (kb->enter) {
            start_match_from_selection();
        }

        if (kb->pause) {
            current_state = STATE_MENU;
            return finish_update(0);
        }

        if (mouse_clicked) {
            int character = character_at_point(cursor_x, cursor_y);

            if (character >= 0) {
                if (!choosing_player2) {
                    selected_player1 = character;
                    selected_player2 = -1;
                    choosing_player2 = true;
                } else if (character != selected_player1) {
                    selected_player2 = character;
                    start_match_from_selection();
                }
            }
        }
    }

    else if (current_state == STATE_FIELD_SELECT) {
        if (kb->pause) {
            current_state = STATE_CHARACTER_SELECT;
            return finish_update(0);
        }

        if (mouse_clicked) {
            t_field_type field = field_at_point(cursor_x, cursor_y);

            if (field < FIELD_COUNT) {
                selected_field = field;
                start_match_from_field_selection();
            }
        }
    }

    else if (current_state == STATE_PLAYING) {
        if (kb->pause) {
            current_state = STATE_PAUSED;
            return finish_update(0);
        }

        update_game_timer();

        if (current_state == STATE_END) {
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
                reset_round(-1);
            } else if (point == -1) {
                score_p2++;
                reset_round(1);
            }
            check_win_after_point();
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

    else if (current_state == STATE_CHARACTER_SELECT) {
        video_draw_character_select(selected_player1, selected_player2, choosing_player2);
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_FIELD_SELECT) {
        draw_field_select_menu();
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_PLAYING) {
        draw_field(selected_field);
        video_draw_hud(score_p1, score_p2, sudden_death ? -sudden_death_seconds : game_seconds_left);
        video_draw_net();
        video_draw_ball(&g_ball);
        video_draw_players(&g_player1, &g_player2);
        if (sudden_death) {
            video_draw_sudden_death_notice(sudden_death_seconds, timer_frame_count);
        }
        
    }

    else if (current_state == STATE_PAUSED) {
        draw_field(selected_field);
        video_draw_hud(score_p1, score_p2, sudden_death ? -sudden_death_seconds : game_seconds_left);
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
