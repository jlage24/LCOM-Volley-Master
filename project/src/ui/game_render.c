#include "game_render.h"
#include "../../../lab5/video.h" 

void video_draw_background() {
    vg_draw_rectangle(0, 0, 1024, 680, 0x87CEEB); //Blue
    
    vg_draw_rectangle(0, 680, 1024, 768 - 680, 0xEEDD82);
}

void video_draw_net() {
    uint16_t net_width = 10;
    uint16_t start_x = 512 - (net_width / 2); 
    uint16_t start_y = 680 - 160;             
    
    vg_draw_rectangle(start_x, start_y, net_width, 160, 0xFFFFFF); //White
}

void video_draw_ball(Ball *b) {
    uint16_t radius = 16;
    uint16_t draw_x = (uint16_t)(b->x) - radius;
    uint16_t draw_y = (uint16_t)(b->y) - radius;
    uint16_t width = radius * 2;
    
    vg_draw_rectangle(draw_x, draw_y, width, width, 0xFFFF00); // Yellow
}

void video_draw_players(Player *p1, Player *p2) {
    uint16_t width = 50;
    uint16_t height = 100;
    
    uint16_t p1_x = (uint16_t)(p1->x) - (width / 2);
    uint16_t p1_y = (uint16_t)(p1->y) - (height / 2);
    vg_draw_rectangle(p1_x, p1_y, width, height, 0xFF0000); //Red
    
    uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
    uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);
    vg_draw_rectangle(p2_x, p2_y, width, height, 0x0000FF); //Blue
}