#ifndef _MENU_H_
#define _MENU_H_

#include "../game/game.h"
#include "game_render.h"

void draw_main_menu();
void draw_pause_menu();
void draw_field(t_field_type field);
void draw_field_select_menu();
void draw_end_screen(int winner);
void draw_how_to_play_screen();

#endif /* _MENU_H_ */
