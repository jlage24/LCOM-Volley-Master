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

/* -------------------------------------------------------- */
/* 7-segment digit renderer for score HUD                   */
/* -------------------------------------------------------- */

/*
 * Segment layout:
 *
 *   -0-
 *  |   |
 *  5   1
 *  |   |
 *   -6-
 *  |   |
 *  4   2
 *  |   |
 *   -3-
 */

/* Which segments are lit for digits 0-9 (bit flags, segment 0 = bit 0). */
static const uint8_t seg_table[10] = {
    /* 0 */ 0x3F, /* segments 0,1,2,3,4,5 */
    /* 1 */ 0x06, /* segments 1,2 */
    /* 2 */ 0x5B, /* segments 0,1,3,4,6 */
    /* 3 */ 0x4F, /* segments 0,1,2,3,6 */
    /* 4 */ 0x66, /* segments 1,2,5,6 */
    /* 5 */ 0x6D, /* segments 0,2,3,5,6 */
    /* 6 */ 0x7D, /* segments 0,2,3,4,5,6 */
    /* 7 */ 0x07, /* segments 0,1,2 */
    /* 8 */ 0x7F, /* segments 0,1,2,3,4,5,6 */
    /* 9 */ 0x6F, /* segments 0,1,2,3,5,6 */
};

/* Segment dimensions. */
#define SEG_W  20   /* horizontal segment width  */
#define SEG_H   4   /* horizontal segment height */
#define SEG_VW  4   /* vertical segment width    */
#define SEG_VH 16   /* vertical segment height   */

static void draw_7seg_digit(uint16_t ox, uint16_t oy, int digit, uint32_t color) {
    if (digit < 0 || digit > 9) return;

    uint8_t segs = seg_table[digit];

    /* Seg 0 – top horizontal */
    if (segs & (1 << 0))
        draw_rectangle_bb(ox + SEG_VW, oy, SEG_W, SEG_H, color);

    /* Seg 1 – upper right vertical */
    if (segs & (1 << 1))
        draw_rectangle_bb(ox + SEG_VW + SEG_W, oy + SEG_H, SEG_VW, SEG_VH, color);

    /* Seg 2 – lower right vertical */
    if (segs & (1 << 2))
        draw_rectangle_bb(ox + SEG_VW + SEG_W, oy + SEG_H + SEG_VH + SEG_H, SEG_VW, SEG_VH, color);

    /* Seg 3 – bottom horizontal */
    if (segs & (1 << 3))
        draw_rectangle_bb(ox + SEG_VW, oy + SEG_H + SEG_VH + SEG_H + SEG_VH, SEG_W, SEG_H, color);

    /* Seg 4 – lower left vertical */
    if (segs & (1 << 4))
        draw_rectangle_bb(ox, oy + SEG_H + SEG_VH + SEG_H, SEG_VW, SEG_VH, color);

    /* Seg 5 – upper left vertical */
    if (segs & (1 << 5))
        draw_rectangle_bb(ox, oy + SEG_H, SEG_VW, SEG_VH, color);

    /* Seg 6 – middle horizontal */
    if (segs & (1 << 6))
        draw_rectangle_bb(ox + SEG_VW, oy + SEG_H + SEG_VH, SEG_W, SEG_H, color);
}

/* Digit glyph total width/height for spacing. */
#define DIGIT_W  (SEG_VW + SEG_W + SEG_VW)  /* 28 px */
#define DIGIT_GAP 6

void video_draw_score(int score_p1, int score_p2) {
    if (score_p1 < 0) score_p1 = 0;
    if (score_p2 < 0) score_p2 = 0;

    int tens, ones;

    /* Player 1 score – left side, red tint. */
    tens = (score_p1 / 10) % 10;
    ones = score_p1 % 10;
    draw_7seg_digit(380, 40, tens, 0xFF4444);
    draw_7seg_digit(380 + DIGIT_W + DIGIT_GAP, 40, ones, 0xFF4444);

    /* Player 2 score – right side, blue tint. */
    tens = (score_p2 / 10) % 10;
    ones = score_p2 % 10;
    draw_7seg_digit(610, 40, tens, 0x4488FF);
    draw_7seg_digit(610 + DIGIT_W + DIGIT_GAP, 40, ones, 0x4488FF);
}

#pragma clang optimize on
