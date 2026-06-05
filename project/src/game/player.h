#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "game.h"

/* Player movement constants. */
#define PLAYER1_SPEED   5.0f
#define PLAYER2_SPEED   5.0f
#define JUMP_IMPULSE   -8.0f
#define PLAYER_WIDTH    120
#define PLAYER_HEIGHT   180

void player1_update(Player *p);
void player2_update(Player *p);
void player_init(Player *p, float start_x);

#endif
