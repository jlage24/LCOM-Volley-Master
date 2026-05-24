#include "game_render.h"
#include "assets.h"
#include "../../../lab5/video.h" 
#include <stdlib.h> 
#include <string.h>

extern char *video_mem;

static char *back_buffer; 
static uint16_t h_res = 1024;
static uint16_t v_res = 768;
static uint8_t bytes_per_pixel = 3;

int video_init_back_buffer() {
    unsigned int vram_size = h_res * v_res * bytes_per_pixel;
    back_buffer = malloc(vram_size);
    
    if (back_buffer == NULL) return 1;
    return 0;
}

void video_free_back_buffer() {
    if (back_buffer != NULL) {
        free(back_buffer);
        back_buffer = NULL;
    }
}

void video_flush() {
    unsigned int vram_size = h_res * v_res * bytes_per_pixel;
    memcpy(video_mem, back_buffer, vram_size);
}

int vg_draw_pixel_bb(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= h_res || y >= v_res) return 0; 
    
    unsigned int offset = (y * h_res + x) * bytes_per_pixel;
    memcpy((uint8_t*)back_buffer + offset, &color, bytes_per_pixel);
    return 0;
}

void draw_rectangle_bb(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    for (uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            unsigned int offset = ((y + i) * h_res + (x + j)) * bytes_per_pixel;
            back_buffer[offset] = color & 0xFF;             // Azul
            back_buffer[offset + 1] = (color >> 8) & 0xFF;  // Verde
            back_buffer[offset + 2] = (color >> 16) & 0xFF; // Vermelho
        }
    }
}

void video_draw_background() {
    if (bg_pixmap != NULL) {
        memcpy(back_buffer, bg_pixmap, 1024 * 768 * 3); 
    } else {
        draw_rectangle_bb(0, 0, 1024, 768, 0x000000); 
    }
}

void video_draw_net() {
    uint16_t net_width = 10;
    uint16_t start_x = 512 - (net_width / 2); 
    uint16_t start_y = 680 - 160;             
    
    draw_rectangle_bb(start_x, start_y, net_width, 160, 0xFFFFFF); // 15 = Branco (Rede)
}

void video_draw_ball(Ball *b) {
    uint16_t radius = 16;
    uint16_t draw_x = (uint16_t)(b->x) - radius;
    uint16_t draw_y = (uint16_t)(b->y) - radius;
    uint16_t width = radius * 2;
    
    draw_rectangle_bb(draw_x, draw_y, width, width, 0x00FF00); // 10 = Verde Claro (Bola - só para não confundir com a areia)
}

void video_draw_players(Player *p1, Player *p2) {
    uint16_t width = 50;
    uint16_t height = 100;
    
    uint16_t p1_x = (uint16_t)(p1->x) - (width / 2);
    uint16_t p1_y = (uint16_t)(p1->y) - (height / 2);
    draw_rectangle_bb(p1_x, p1_y, width, height, 0xFF0000); // 4 = Vermelho (Player 1)
    
    uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
    uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);
    draw_rectangle_bb(p2_x, p2_y, width, height, 0x0000FF); // 1 = Azul Escuro (Player 2)
}

