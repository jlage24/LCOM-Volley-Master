#ifndef _GAME_RENDER_H_
#define _GAME_RENDER_H_

#include <stdint.h>
#include <lcom/lcf.h>
#include "../game/game.h" 

int video_init_back_buffer();
void video_free_back_buffer();
void video_flush();

int vg_draw_pixel_bb(uint16_t x, uint16_t y, uint32_t color);
void draw_rectangle_bb(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

void video_draw_net();
void video_draw_ball(Ball *b);
void video_draw_players(Player *p1, Player *p2);
void video_draw_hud(int score_left, int score_right, int seconds_left);
void video_draw_cursor(int x, int y);
void draw_xpm(xpm_image_t img, uint16_t x, uint16_t y);
void video_draw_score(int score_p1, int score_p2);


#endif /* _GAME_RENDER_H_ */
