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
#define MENU_BUTTON_SELECT_X_OFFSET 13
#define MENU_BUTTON_SELECT_Y_OFFSET 38
#define MENU_BUTTON_SELECT_W 274
#define MENU_BUTTON_SELECT_H 53
#define FIELD_CARD_Y 388
#define FIELD_CARD_W 190
#define FIELD_CARD_H 205
#define COUNTDOWN_NUMBER_TICKS 60
#define COUNTDOWN_START_TICKS 45

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
static int countdown_ticks = 0;
static int winner = 0;
static int cursor_x = H_RES / 2;
static int cursor_y = V_RES / 2;
static bool mouse_button_was_down = false;
static bool should_exit = false;
static bool sudden_death = false;
static int selected_player1 = -1;
static int selected_player2 = -1;
static bool choosing_player2 = false;
static int main_menu_selected = 0;
static int pause_menu_selected = 0;
static int character_selected_index = 0;
static int field_selected_index = FIELD_JUNGLE;
static int end_menu_selected = 0;
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
    countdown_ticks = 0;
    sudden_death = false;
    winner = 0;
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

static int first_available_character(int unavailable_character);
static void start_match_from_selection(void);
static void start_match_from_field_selection(void);

static void move_vertical_selection(int *selected, int count, const KeyboardState *kb) {
    if (kb->up) {
        *selected = (*selected + count - 1) % count;
    }
    if (kb->down) {
        *selected = (*selected + 1) % count;
    }
}

static void move_horizontal_selection(int *selected, int count, const KeyboardState *kb) {
    if (kb->left) {
        *selected = (*selected + count - 1) % count;
    }
    if (kb->right) {
        *selected = (*selected + 1) % count;
    }
}

static void move_character_selection(const KeyboardState *kb) {
    int direction = 0;

    if (kb->left) direction--;
    if (kb->right) direction++;

    if (direction == 0) {
        return;
    }

    do {
        character_selected_index =
            (character_selected_index + PLAYER_CHARACTER_COUNT + direction) %
            PLAYER_CHARACTER_COUNT;
    } while (choosing_player2 && character_selected_index == selected_player1);
}

static int button_at_point(int x, int y) {
    if (point_inside_rect(x, y,
                          MENU_BUTTON_X, MENU_EXIT_Y,
                          MENU_BUTTON_W, MENU_BUTTON_H)) {
        return 1;
    }
    if (point_inside_rect(x, y,
                          MENU_BUTTON_X, MENU_START_Y,
                          MENU_BUTTON_W, MENU_BUTTON_H)) {
        return 0;
    }

    return -1;
}

static int sync_button_selection_from_mouse(int *selected) {
    int hovered_button = button_at_point(cursor_x, cursor_y);

    if (hovered_button >= 0) {
        *selected = hovered_button;
    }

    return hovered_button;
}

static void draw_selected_menu_button(int selected) {
    video_draw_selection_box(MENU_BUTTON_X + MENU_BUTTON_SELECT_X_OFFSET,
                             (selected == 0 ? MENU_START_Y : MENU_EXIT_Y) +
                             MENU_BUTTON_SELECT_Y_OFFSET,
                             MENU_BUTTON_SELECT_W,
                             MENU_BUTTON_SELECT_H);
}

static void choose_current_character(void) {
    if (!choosing_player2) {
        selected_player1 = character_selected_index;
        selected_player2 = -1;
        character_selected_index = first_available_character(selected_player1);
        choosing_player2 = true;
    } else if (character_selected_index != selected_player1) {
        selected_player2 = character_selected_index;
        start_match_from_selection();
    }
}

static void choose_current_field(void) {
    selected_field = (t_field_type)field_selected_index;
    start_match_from_field_selection();
}

static void open_character_select(void) {
    selected_player1 = -1;
    selected_player2 = -1;
    selected_field = FIELD_JUNGLE;
    character_selected_index = 0;
    field_selected_index = FIELD_JUNGLE;
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
    field_selected_index = selected_field;
    current_state = STATE_FIELD_SELECT;
}

static void start_match_from_field_selection(void) {
    reset_match();
    current_state = STATE_COUNTDOWN;
}

static int countdown_display_value(void) {
    if (countdown_ticks < COUNTDOWN_NUMBER_TICKS) {
        return 3;
    }
    if (countdown_ticks < COUNTDOWN_NUMBER_TICKS * 2) {
        return 2;
    }
    if (countdown_ticks < COUNTDOWN_NUMBER_TICKS * 3) {
        return 1;
    }

    return 0;
}

static void finish_match(int winning_player) {
    winner = winning_player;
    end_menu_selected = 0;
    current_state = STATE_END;
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
            finish_match(score_p1 > score_p2 ? 1 : 2);
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
    countdown_ticks = 0;
    cursor_x = H_RES / 2;
    cursor_y = V_RES / 2;
    mouse_button_was_down = false;
    should_exit = false;
    sudden_death = false;
    winner = 0;
    selected_player1 = -1;
    selected_player2 = -1;
    selected_field = FIELD_JUNGLE;
    main_menu_selected = 0;
    pause_menu_selected = 0;
    character_selected_index = 0;
    field_selected_index = FIELD_JUNGLE;
    end_menu_selected = 0;
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
        int hovered_button = sync_button_selection_from_mouse(&main_menu_selected);
        move_vertical_selection(&main_menu_selected, 2, kb);

        if (kb->enter) {
            if (main_menu_selected == 0) {
                open_character_select();
            } else {
                should_exit = true;
                return finish_update(1);
            }
        }

        if (kb->pause) {
            should_exit = true;
            return finish_update(1);
        }

        if (current_state == STATE_MENU && mouse_clicked && hovered_button >= 0) {
            if (main_menu_selected == 1) {
                should_exit = true;
                return finish_update(1);
            }
            else if (main_menu_selected == 0) {
                open_character_select();
            }
        }
    }

    else if (current_state == STATE_CHARACTER_SELECT) {
        int hovered_character = character_at_point(cursor_x, cursor_y);

        if (hovered_character >= 0 &&
            (!choosing_player2 || hovered_character != selected_player1)) {
            character_selected_index = hovered_character;
        }

        move_character_selection(kb);

        if (kb->enter) {
            choose_current_character();
        }

        if (kb->pause) {
            current_state = STATE_MENU;
            return finish_update(0);
        }

        if (mouse_clicked) {
            if (hovered_character >= 0 &&
                (!choosing_player2 || hovered_character != selected_player1)) {
                choose_current_character();
            }
        }
    }

    else if (current_state == STATE_FIELD_SELECT) {
        t_field_type hovered_field = field_at_point(cursor_x, cursor_y);

        if (hovered_field < FIELD_COUNT) {
            field_selected_index = hovered_field;
            selected_field = hovered_field;
        }

        move_horizontal_selection(&field_selected_index, FIELD_COUNT, kb);
        selected_field = (t_field_type)field_selected_index;

        if (kb->enter) {
            choose_current_field();
        }

        if (kb->pause) {
            current_state = STATE_CHARACTER_SELECT;
            return finish_update(0);
        }

        if (mouse_clicked) {
            if (hovered_field < FIELD_COUNT) {
                choose_current_field();
            }
        }
    }

    else if (current_state == STATE_COUNTDOWN) {
        if (kb->pause) {
            current_state = STATE_FIELD_SELECT;
            return finish_update(0);
        }

        countdown_ticks++;

        if (countdown_ticks >= (COUNTDOWN_NUMBER_TICKS * 3) + COUNTDOWN_START_TICKS) {
            current_state = STATE_PLAYING;
        }
    }

    else if (current_state == STATE_PLAYING) {
        if (kb->pause) {
            pause_menu_selected = 0;
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
                if (sudden_death || score_p1 >= WIN_SCORE) {
                    finish_match(1);
                } else {
                    reset_round(-1);
                }
            } else if (point == -1) {
                score_p2++;
                if (sudden_death || score_p2 >= WIN_SCORE) {
                    finish_match(2);
                } else {
                    reset_round(1);
                }
            }
        }
    }

    else if (current_state == STATE_PAUSED) {
        int hovered_button = sync_button_selection_from_mouse(&pause_menu_selected);
        move_vertical_selection(&pause_menu_selected, 2, kb);

        if (kb->enter) {
            if (pause_menu_selected == 0) {
                current_state = STATE_PLAYING;
            } else {
                current_state = STATE_MENU;
            }
        }

        if (kb->pause) {
            current_state = STATE_MENU;
        }

        if (current_state == STATE_PAUSED && mouse_clicked && hovered_button >= 0) {
            if (pause_menu_selected == 1) {
                current_state = STATE_MENU;
            }
            else if (pause_menu_selected == 0) {
                current_state = STATE_PLAYING;
            }
        }
    }

    else if (current_state == STATE_END) {
        int hovered_button = sync_button_selection_from_mouse(&end_menu_selected);
        move_vertical_selection(&end_menu_selected, 2, kb);

        if (kb->enter) {
            if (end_menu_selected == 0) {
                reset_match();
                current_state = STATE_COUNTDOWN;
            } else {
                reset_match();
                current_state = STATE_MENU;
            }
        }

        if (kb->pause) {
            reset_match();
            current_state = STATE_MENU;
            return finish_update(0);
        }

        if (mouse_clicked && hovered_button >= 0) {
            if (end_menu_selected == 1) {
                reset_match();
                current_state = STATE_MENU;
            }
            else if (end_menu_selected == 0) {
                reset_match();
                current_state = STATE_COUNTDOWN;
            }
        }
    }

    return finish_update(0);
}

void game_render(void) {
    if (current_state == STATE_MENU) {
        draw_main_menu();
        draw_selected_menu_button(main_menu_selected);
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_CHARACTER_SELECT) {
        video_draw_character_select(selected_player1, selected_player2, choosing_player2);
        video_draw_selection_box(character_card_x[character_selected_index],
                                 CHARACTER_CARD_Y,
                                 CHARACTER_CARD_W, CHARACTER_CARD_H);
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_FIELD_SELECT) {
        draw_field_select_menu();
        video_draw_selection_box(field_card_x[field_selected_index], FIELD_CARD_Y,
                                 FIELD_CARD_W, FIELD_CARD_H);
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_COUNTDOWN) {
        draw_field(selected_field);
        video_draw_hud(score_p1, score_p2, game_seconds_left);
        video_draw_net();
        video_draw_ball(&g_ball);
        video_draw_players(&g_player1, &g_player2);
        video_draw_countdown(countdown_display_value());
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
        draw_selected_menu_button(pause_menu_selected);
        video_draw_cursor(cursor_x, cursor_y);
    }

    else if (current_state == STATE_END) {
        draw_end_screen(winner);
        video_draw_end_overlay(score_p1, score_p2);
        draw_selected_menu_button(end_menu_selected);
        video_draw_cursor(cursor_x, cursor_y);
    }

    video_flush();
}

bool game_should_exit(void) {
    return should_exit;
}
