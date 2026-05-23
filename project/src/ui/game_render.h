#ifndef _GAME_RENDER_H_
#define _GAME_RENDER_H_

#include "../game/game.h" 

void video_draw_background();
void video_draw_net();
void video_draw_ball(Ball *b);
void video_draw_players(Player *p1, Player *p2);

#endif /* _GAME_RENDER_H_ */