#include "game_render.h"
#include "assets.h"
#include "../game/player.h"
#include "../../../lab5/video.h" 
#include <stdio.h>
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
    draw_xpm(player1_pixmap, p1_x, p1_y);
    
    uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
    uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);
    draw_xpm(player2_pixmap, p2_x, p2_y);
}

static uint8_t glyph_row(char c, int row) {
    static const uint8_t digits[10][5] = {
        {7, 5, 5, 5, 7},
        {2, 6, 2, 2, 7},
        {7, 1, 7, 4, 7},
        {7, 1, 7, 1, 7},
        {5, 5, 7, 1, 1},
        {7, 4, 7, 1, 7},
        {7, 4, 7, 5, 7},
        {7, 1, 1, 1, 1},
        {7, 5, 7, 5, 7},
        {7, 5, 7, 1, 7}
    };
    static const uint8_t colon[5] = {0, 2, 0, 2, 0};

    if (c >= '0' && c <= '9') {
        return digits[c - '0'][row];
    }
    if (c == ':') {
        return colon[row];
    }
    return 0;
}

static uint16_t text_width(const char *text, uint16_t scale) {
    uint16_t width = 0;

    for (size_t i = 0; text[i] != '\0'; i++) {
        width += (text[i] == ' ') ? (scale * 2) : (scale * 4);
    }

    return width;
}

static void draw_char(char c, uint16_t x, uint16_t y, uint16_t scale, uint32_t color) {
    for (int row = 0; row < 5; row++) {
        uint8_t bits = glyph_row(c, row);

        for (int col = 0; col < 3; col++) {
            if (bits & BIT(2 - col)) {
                draw_rectangle_bb(x + (col * scale), y + (row * scale),
                                  scale, scale, color);
            }
        }
    }
}

static void draw_text(const char *text, uint16_t x, uint16_t y,
                      uint16_t scale, uint32_t color) {
    uint16_t cursor = x;

    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] != ' ') {
            draw_char(text[i], cursor, y, scale, color);
        }
        cursor += (text[i] == ' ') ? (scale * 2) : (scale * 4);
    }
}

void video_draw_hud(int score_left, int score_right, int seconds_left) {
    char hud[20];
    uint16_t scale = 5;
    uint16_t y = 24;

    if (seconds_left < 0) seconds_left = 0;
    snprintf(hud, sizeof(hud), "%d   %02d:%02d   %d",
             score_left, seconds_left / 60, seconds_left % 60, score_right);

    uint16_t x = (h_res - text_width(hud, scale)) / 2;
    draw_text(hud, x + 2, y + 2, scale, 0x000000);
    draw_text(hud, x, y, scale, 0xFFFFFF);
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
