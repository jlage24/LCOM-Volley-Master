#include "game_render.h"
#include "assets.h"
#include "../game/player.h"
#include "../../../lab5/video.h" 
#include <stdlib.h> 
#include <string.h>

extern char *video_mem;

char *back_buffer; 
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

#pragma clang optimize off

int vg_draw_pixel_bb(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= h_res || y >= v_res) return 0; 
    
    unsigned int offset = (y * h_res + x) * bytes_per_pixel;
    uint8_t *pixel = (uint8_t*)back_buffer + offset;
    
    pixel[0] = color & 0xFF;             // Blue
    pixel[1] = (color >> 8) & 0xFF;      // Green
    pixel[2] = (color >> 16) & 0xFF;     // Red
    
    return 0;
}

void draw_rectangle_bb(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    for (uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            vg_draw_pixel_bb(x + j, y + i, color);
        }
    }
}


void video_draw_net() {
    uint16_t net_width = 10;
    uint16_t start_x = 512 - (net_width / 2); 
    uint16_t start_y = 680 - 160;             
    
    draw_rectangle_bb(start_x, start_y, net_width, 160, 0xFFFFFF); // White net
}

void video_draw_ball(Ball *b) {
    uint16_t radius = 16;
    uint16_t draw_x = (uint16_t)(b->x) - radius;
    uint16_t draw_y = (uint16_t)(b->y) - radius;
    uint16_t width = radius * 2;
    
    draw_rectangle_bb(draw_x, draw_y, width, width, 0x00FF00); // Light green ball
}

void video_draw_players(Player *p1, Player *p2) {
    uint16_t width = PLAYER_WIDTH;
    uint16_t height = PLAYER_HEIGHT;
    
    uint16_t p1_x = (uint16_t)(p1->x) - (width / 2);
    uint16_t p1_y = (uint16_t)(p1->y) - (height / 2);
    draw_rectangle_bb(p1_x, p1_y, width, height, 0xFF0000); // Red player 1
    
    uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
    uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);
    draw_rectangle_bb(p2_x, p2_y, width, height, 0x0000FF); // Blue player 2
}

static void draw_cursor_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= h_res || y >= v_res) {
        return;
    }

    vg_draw_pixel_bb((uint16_t)x, (uint16_t)y, color);
}

void video_draw_cursor(int x, int y) {
    const uint32_t outline = 0x000000;
    const uint32_t fill = 0xFFFFFF;

    for (int i = 0; i < 18; i++) {
        draw_cursor_pixel(x, y + i, outline);
        draw_cursor_pixel(x + 1, y + i, fill);
    }

    for (int i = 0; i < 12; i++) {
        draw_cursor_pixel(x + i, y + i, outline);
        draw_cursor_pixel(x + i + 1, y + i, fill);
    }

    for (int i = 0; i < 7; i++) {
        draw_cursor_pixel(x + 5 + i, y + 12 + i, outline);
        draw_cursor_pixel(x + 6 + i, y + 12 + i, fill);
    }

    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 4; col++) {
            draw_cursor_pixel(x + 6 + col, y + 13 + row, fill);
        }
    }
}

void draw_xpm(xpm_image_t img, uint16_t x, uint16_t y) {
    uint32_t *map = (uint32_t *)img.bytes;
    uint32_t transparent = xpm_transparency_color(img.type);
    
    for (uint16_t i = 0; i < img.height; i++) {
        for (uint16_t j = 0; j < img.width; j++) {
            uint32_t color = map[i * img.width + j];
            if (color != transparent) { 
                vg_draw_pixel_bb(x + j, y + i, color); 
            }
        }
    }
}
#pragma clang optimize on
