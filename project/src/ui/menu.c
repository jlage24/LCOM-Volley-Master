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

void draw_field_select_menu() {
    draw_xpm(field_select_pixmap, 0, 0);
}

void draw_end_screen(t_winner winner) {
    if (winner == WINNER_PLAYER1) {
        draw_xpm(player1_wins_pixmap, 0, 0);
    } else if (winner == WINNER_PLAYER2) {
        draw_xpm(player2_wins_pixmap, 0, 0);
    }
}

void draw_field(t_field_type field) {
    switch (field) {
        case FIELD_BEACH:
            draw_xpm(beach_volley_pixmap, 0, 0);
            break;
        case FIELD_DESERT:
            draw_xpm(desert_volley_pixmap, 0, 0);
            break;
        case FIELD_ICE:
            draw_xpm(ice_volley_pixmap, 0, 0);
            break;
        case FIELD_JUNGLE:
        default:
            draw_xpm(jungle_volley_pixmap, 0, 0);
            break;
    }
}





