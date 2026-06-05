/*
 * Player movement and per-frame physics.
 *
 * Player 1 is controlled by keyboard state, Player 2 by mouse movement/button
 * state. The game loop calls these functions once per timer frame before ball
 * collision checks, so velocity is available for momentum transfer.
 */
#include "player.h"
#include "../devices/keyboard.h"
#include "../devices/mouse.h"
#include "game.h"

void player1_update(Player *p) {
    const KeyboardState *kb = get_keyboard_state();
    float previous_x = p->x;

    /* Keyboard movement is continuous while keys are held. */
    p->vx = 0;
    if (kb->move_left)  { p->x -= PLAYER1_SPEED; p->vx = -PLAYER1_SPEED; }
    if (kb->move_right) { p->x += PLAYER1_SPEED; p->vx =  PLAYER1_SPEED; }

    /* Player 1 is constrained to the left half of the court. */
    if (p->x < PLAYER_WIDTH / 2)        p->x = PLAYER_WIDTH / 2;
    if (p->x > NET_X - PLAYER_WIDTH / 2) p->x = NET_X - PLAYER_WIDTH / 2;

    if (p->x > previous_x) p->facing_right = true;
    else if (p->x < previous_x) p->facing_right = false;

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
}

void player2_update(Player *p) {
    const float mouse_sensitivity = 1.2f;
    float previous_x = p->x;

    /* Mouse deltas are per-frame input, clamped to the same gameplay scale. */
    float movement = (float)mouse_state.dx * mouse_sensitivity;
    if (movement > PLAYER2_SPEED)  movement = PLAYER2_SPEED;
    if (movement < -PLAYER2_SPEED) movement = -PLAYER2_SPEED;

    p->vx = movement;
    p->x += movement;

    /* Player 2 is constrained to the right half of the court. */
    if (p->x < NET_X + PLAYER_WIDTH / 2) p->x = NET_X + PLAYER_WIDTH / 2;
    if (p->x > H_RES - PLAYER_WIDTH / 2) p->x = H_RES - PLAYER_WIDTH / 2;

    if (p->x > previous_x) p->facing_right = true;
    else if (p->x < previous_x) p->facing_right = false;

    if (mouse_state.left_button && p->on_ground) {
        p->vy = JUMP_IMPULSE;
        p->on_ground = false;
    }

    /* Shared vertical physics: gravity until the player lands on the floor. */
    p->vy += GRAVITY;
    p->y  += p->vy;

    if (p->y >= FLOOR_Y - PLAYER_HEIGHT / 2) {
        p->y = FLOOR_Y - PLAYER_HEIGHT / 2;
        p->vy = 0;
        p->on_ground = true;
    }
}

void player_init(Player *p, float start_x) {
    p->x = start_x;
    p->y = FLOOR_Y - PLAYER_HEIGHT / 2;
    p->vx = 0;
    p->vy = 0;
    p->facing_right = start_x <= NET_X;
    p->on_ground = true;
    p->score = 0;
}
