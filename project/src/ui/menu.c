#include "menu.h"
#include "assets.h"
#include <string.h>

extern char *back_buffer;

void draw_main_menu() {

    draw_xpm(menu_pixmap, 0, 0);

    int pos_x = 80; 
    int pos_y_start = 380; 
    int pos_y_exit = 460; 
    
    draw_xpm(btn_start_pixmap, pos_x, pos_y_start);
    draw_xpm(btn_exit_pixmap, pos_x, pos_y_exit);
}


void draw_pause_menu() {
    
    draw_xpm(menu_pause_pixmap, 0, 0);

    int pos_x = 80; 
    int pos_y_start = 380; 
    int pos_y_exit = 460; 
    
    draw_xpm(btn_resume_pixmap, pos_x, pos_y_start);
    draw_xpm(btn_menu_pixmap, pos_x, pos_y_exit);
}

void draw_field() {
    draw_xpm(jungle_volley_pixmap, 0, 0);
}







