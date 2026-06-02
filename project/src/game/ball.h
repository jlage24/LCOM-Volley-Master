#ifndef _BALL_H_
#define _BALL_H_

#include "game.h"
#include "player.h"

void ball_init(Ball *b, int direction);
int ball_update(Ball *b);
void ball_collide_with_player(Ball *b, const Player *p, int direction);

#endif
