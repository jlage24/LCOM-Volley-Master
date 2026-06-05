#ifndef _PLAYER_H_
#define _PLAYER_H_

/*
 * Player movement interface.
 *
 * game.c updates both players once per timer frame. Player 1 reads keyboard
 * state, Player 2 reads mouse state, and both expose velocity for ball rebounds.
 */
#include "game.h"

/* Movement constants are tuned in pixels per frame for the 60 Hz loop. */
#define PLAYER1_SPEED   5.0f
#define PLAYER2_SPEED   8.0f
#define JUMP_IMPULSE   -8.0f
#define PLAYER_WIDTH    120
#define PLAYER_HEIGHT   180

void player1_update(Player *p);
void player2_update(Player *p);
/* start_x determines court side and initial facing direction. */
void player_init(Player *p, float start_x);

#endif
