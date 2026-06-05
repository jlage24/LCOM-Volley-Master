#ifndef _MENU_H_
#define _MENU_H_

/*
 * Menu and field-screen drawing API.
 *
 * These functions draw static screen assets only. game.c owns navigation,
 * selected indexes and transitions.
 */
#include "../game/game.h"
#include "game_render.h"

void draw_main_menu();
void draw_pause_menu();
void draw_field(t_field_type field);
void draw_field_select_menu();
void draw_end_screen(int winner);
void draw_end_menu_buttons();
void draw_how_to_play_screen();

#endif /* _MENU_H_ */
