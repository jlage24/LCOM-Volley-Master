#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "game.h"

/* Player characteristics and ingame logic*/
#define PLAYER1_SPEED   5.0f
#define JUMP_IMPULSE   -8.0f
#define PLAYER_WIDTH    32
#define PLAYER_HEIGHT   64

void player1_update(Player *p);
void player_init(Player *p, float start_x);

#endif

