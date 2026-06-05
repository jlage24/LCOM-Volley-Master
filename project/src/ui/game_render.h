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
void video_draw_selection_box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void video_draw_net();
void video_draw_ball(Ball *b);
void video_draw_players(Player *p1, Player *p2);
void video_draw_hud(int score_left, int score_right, int seconds_left);
void video_draw_end_overlay(int score_left, int score_right);
void video_draw_countdown(int value);
void video_draw_sudden_death_notice(int seconds, int frame_count);
void video_draw_character_select(int selected_p1, int selected_p2, bool choosing_p2);
void video_draw_cursor(int x, int y);
void draw_xpm(xpm_image_t img, uint16_t x, uint16_t y);
void draw_xpm_flipped_horizontal(xpm_image_t img, uint16_t x, uint16_t y);


#endif /* _GAME_RENDER_H_ */
