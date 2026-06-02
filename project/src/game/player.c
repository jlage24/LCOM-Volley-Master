#include "player.h"
#include "../devices/keyboard.h"
#include "game.h"

void player1_update(Player *p) {
    const KeyboardState *kb = get_keyboard_state();

    if (kb->move_left)  p->x -= PLAYER1_SPEED;
    if (kb->move_right) p->x += PLAYER1_SPEED;

    if (p->x < PLAYER_WIDTH / 2)        p->x = PLAYER_WIDTH / 2;
    if (p->x > NET_X - PLAYER_WIDTH / 2) p->x = NET_X - PLAYER_WIDTH / 2;

    /* Jump only if the player is on the ground. */
    if (kb->jump && p->on_ground) {
        p->vy = JUMP_IMPULSE;
        p->on_ground = false;
    }

    p->vy += GRAVITY;
    p->y  += p->vy;

    if (p->y >= FLOOR_Y - PLAYER_HEIGHT / 2) {
        p->y = FLOOR_Y - PLAYER_HEIGHT / 2;
        p->vy = 0;
        p->on_ground = true;
    }

    keyboard_clear_events();
}

void player_init(Player *p, float start_x) {
    p->x = start_x;
    p->y = FLOOR_Y - PLAYER_HEIGHT / 2;
    p->vy = 0;
    p->on_ground = true;
    p->score = 0;
}
