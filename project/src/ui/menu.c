#include "menu.h"

void draw_main_menu() {
    draw_rectangle_bb(0, 0, 1024, 768, 8); 
    
    draw_rectangle_bb(512 - 100, 384 - 50, 200, 100, 2); 
}

void draw_pause_menu() {
    draw_rectangle_bb(512 - 200, 384 - 150, 400, 300, 20); 
}

void draw_game_over_menu() {
    draw_rectangle_bb(0, 0, 1024, 768, 4); 
}


