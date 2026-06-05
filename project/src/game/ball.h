#ifndef _BALL_H_
#define _BALL_H_

/*
 * Ball physics interface.
 *
 * game.c owns the Ball instance and calls these functions during active play.
 * ball_update reports scoring, while player collision mutates ball velocity.
 */
#include "game.h"
#include "player.h"

/* direction selects the initial horizontal serve direction; zero defaults right. */
void ball_init(Ball *b, int direction);
/* Returns -1 for a floor hit on the left side, +1 on the right side, or 0. */
int ball_update(Ball *b);
void ball_collide_with_player(Ball *b, const Player *p, int direction);

#endif
