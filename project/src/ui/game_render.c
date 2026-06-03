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

static void blend_rectangle_bb(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                               uint8_t keep_percent) {
    for (uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            uint16_t px = x + j;
            uint16_t py = y + i;

            if (px >= h_res || py >= v_res) {
                continue;
            }

            unsigned int offset = (py * h_res + px) * bytes_per_pixel;
            uint8_t *pixel = (uint8_t*)back_buffer + offset;

            pixel[0] = (uint8_t)((pixel[0] * keep_percent) / 100);
            pixel[1] = (uint8_t)((pixel[1] * keep_percent) / 100);
            pixel[2] = (uint8_t)((pixel[2] * keep_percent) / 100);
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
    if (player1_pixmap != NULL) {
        if (p1->facing_right) draw_xpm(*player1_pixmap, p1_x, p1_y);
        else draw_xpm_flipped_horizontal(*player1_pixmap, p1_x, p1_y);
    }
    
    uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
    uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);
    if (player2_pixmap != NULL) {
        if (p2->facing_right) draw_xpm(*player2_pixmap, p2_x, p2_y);
        else draw_xpm_flipped_horizontal(*player2_pixmap, p2_x, p2_y);
    }
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
    static const uint8_t plus[5] = {0, 2, 7, 2, 0};

    if (c >= '0' && c <= '9') {
        return digits[c - '0'][row];
    }
    if (c == ':') {
        return colon[row];
    }
    if (c == '+') {
        return plus[row];
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

static uint8_t wide_glyph_row(char c, int row) {
    static const uint8_t letters[26][7] = {
        {14, 17, 17, 31, 17, 17, 17}, // A
        {30, 17, 17, 30, 17, 17, 30}, // B
        {14, 17, 16, 16, 16, 17, 14}, // C
        {30, 17, 17, 17, 17, 17, 30}, // D
        {31, 16, 16, 30, 16, 16, 31}, // E
        {31, 16, 16, 30, 16, 16, 16}, // F
        {14, 17, 16, 19, 17, 17, 14}, // G
        {17, 17, 17, 31, 17, 17, 17}, // H
        {14, 4, 4, 4, 4, 4, 14},      // I
        {7, 2, 2, 2, 2, 18, 12},      // J
        {17, 18, 20, 24, 20, 18, 17}, // K
        {16, 16, 16, 16, 16, 16, 31}, // L
        {17, 27, 21, 21, 17, 17, 17}, // M
        {17, 25, 21, 19, 17, 17, 17}, // N
        {14, 17, 17, 17, 17, 17, 14}, // O
        {30, 17, 17, 30, 16, 16, 16}, // P
        {14, 17, 17, 17, 21, 18, 13}, // Q
        {30, 17, 17, 30, 20, 18, 17}, // R
        {15, 16, 16, 14, 1, 1, 30},   // S
        {31, 4, 4, 4, 4, 4, 4},       // T
        {17, 17, 17, 17, 17, 17, 14}, // U
        {17, 17, 17, 17, 17, 10, 4},  // V
        {17, 17, 17, 21, 21, 21, 10}, // W
        {17, 17, 10, 4, 10, 17, 17},  // X
        {17, 17, 10, 4, 4, 4, 4},     // Y
        {31, 1, 2, 4, 8, 16, 31}      // Z
    };

    if (c >= 'a' && c <= 'z') {
        c -= 'a' - 'A';
    }

    if (c >= 'A' && c <= 'Z') {
        return letters[c - 'A'][row];
    }

    return 0;
}

static uint16_t wide_text_width(const char *text, uint16_t scale) {
    uint16_t width = 0;

    for (size_t i = 0; text[i] != '\0'; i++) {
        width += (text[i] == ' ') ? (scale * 4) : (scale * 6);
    }

    return width;
}

static void draw_wide_char(char c, uint16_t x, uint16_t y,
                           uint16_t scale, uint32_t color) {
    for (int row = 0; row < 7; row++) {
        uint8_t bits = wide_glyph_row(c, row);

        for (int col = 0; col < 5; col++) {
            if (bits & BIT(4 - col)) {
                draw_rectangle_bb(x + (col * scale), y + (row * scale),
                                  scale, scale, color);
            }
        }
    }
}

static void draw_wide_text(const char *text, uint16_t x, uint16_t y,
                           uint16_t scale, uint32_t color) {
    uint16_t cursor = x;

    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] != ' ') {
            draw_wide_char(text[i], cursor, y, scale, color);
        }
        cursor += (text[i] == ' ') ? (scale * 4) : (scale * 6);
    }
}

static void draw_centered_wide_text(const char *text, uint16_t y,
                                    uint16_t scale, uint32_t color) {
    uint16_t x = (h_res - wide_text_width(text, scale)) / 2;

    draw_wide_text(text, x + 3, y + 3, scale, 0x000000);
    draw_wide_text(text, x, y, scale, color);
}

static void draw_leaf_block(int x, int y, uint32_t color) {
    draw_rectangle_bb((uint16_t)x, (uint16_t)(y + 3), 10, 5, 0x173C16);
    draw_rectangle_bb((uint16_t)(x + 2), (uint16_t)(y + 1), 6, 10, color);
    draw_rectangle_bb((uint16_t)(x + 4), (uint16_t)(y + 2), 2, 7, 0x8EAE3D);
}

static void draw_vine_between(int x0, int y0, int x1, int y1) {
    const int steps = 22;

    for (int i = 0; i <= steps; i++) {
        int x = x0 + ((x1 - x0) * i) / steps;
        int y = y0 + ((y1 - y0) * i) / steps;

        draw_rectangle_bb((uint16_t)(x - 2), (uint16_t)(y - 2), 5, 5, 0x173C16);
        draw_rectangle_bb((uint16_t)(x - 1), (uint16_t)(y - 1), 3, 3, 0x2D6D25);

        if (i % 7 == 3) {
            draw_leaf_block(x - 10, y - 5, 0x3E8A31);
        } else if (i % 7 == 6) {
            draw_leaf_block(x + 3, y - 4, 0x2F7628);
        }
    }
}

static void draw_leaf_lock(int x, int y) {
    draw_rectangle_bb((uint16_t)(x + 13), (uint16_t)y, 26, 6, 0x102510);
    draw_rectangle_bb((uint16_t)(x + 7), (uint16_t)(y + 6), 6, 22, 0x102510);
    draw_rectangle_bb((uint16_t)(x + 39), (uint16_t)(y + 6), 6, 22, 0x102510);
    draw_rectangle_bb((uint16_t)(x + 13), (uint16_t)(y + 6), 26, 6, 0x4E8A2E);
    draw_rectangle_bb((uint16_t)(x + 13), (uint16_t)(y + 12), 6, 16, 0x4E8A2E);
    draw_rectangle_bb((uint16_t)(x + 33), (uint16_t)(y + 12), 6, 16, 0x4E8A2E);

    draw_rectangle_bb((uint16_t)(x + 2), (uint16_t)(y + 26), 48, 38, 0x0B1A0B);
    draw_rectangle_bb((uint16_t)(x + 7), (uint16_t)(y + 31), 38, 28, 0x2F6D23);
    draw_rectangle_bb((uint16_t)(x + 12), (uint16_t)(y + 36), 28, 18, 0x3E8A31);
    draw_leaf_block(x + 4, y + 30, 0x4E9A36);
    draw_leaf_block(x + 38, y + 30, 0x3E8A31);
    draw_leaf_block(x + 10, y + 50, 0x3B7E2C);
    draw_leaf_block(x + 32, y + 50, 0x4E9A36);

    draw_rectangle_bb((uint16_t)(x + 21), (uint16_t)(y + 43), 10, 8, 0x071007);
    draw_rectangle_bb((uint16_t)(x + 23), (uint16_t)(y + 50), 6, 8, 0x071007);
}

static void draw_locked_character_overlay(uint16_t x, uint16_t y) {
    int portrait_x = x + 12;
    int portrait_y = y + 18;
    int portrait_w = CHARACTER_CARD_W - 24;
    int portrait_h = 144;
    int center_x = portrait_x + portrait_w / 2;
    int center_y = portrait_y + portrait_h / 2;

    blend_rectangle_bb((uint16_t)portrait_x, (uint16_t)portrait_y,
                       (uint16_t)portrait_w, (uint16_t)portrait_h, 42);
    draw_vine_between(portrait_x + 4, portrait_y + 6, center_x, center_y);
    draw_vine_between(portrait_x + portrait_w - 4, portrait_y + 6, center_x, center_y);
    draw_vine_between(portrait_x + 4, portrait_y + portrait_h - 6, center_x, center_y);
    draw_vine_between(portrait_x + portrait_w - 4, portrait_y + portrait_h - 6, center_x, center_y);
    draw_leaf_lock(center_x - 26, center_y - 28);
}

static void draw_character_selection_marker(uint16_t x, uint16_t y, uint32_t color) {
    draw_rectangle_bb(x, y, CHARACTER_CARD_W, 3, color);
    draw_rectangle_bb(x, y + CHARACTER_CARD_H - 3, CHARACTER_CARD_W, 3, color);
    draw_rectangle_bb(x, y, 3, CHARACTER_CARD_H, color);
    draw_rectangle_bb(x + CHARACTER_CARD_W - 3, y, 3, CHARACTER_CARD_H, color);
}

void video_draw_character_select(int selected_p1, int selected_p2, bool choosing_p2) {
    draw_xpm(character_select_pixmap, 0, 0);

    if (choosing_p2) {
        draw_rectangle_bb(350, 142, 330, 56, 0x4B260C);
        draw_centered_wide_text("PLAYER TWO", 154, 5, 0xF2B51F);
    }

    for (int i = 0; i < PLAYER_CHARACTER_COUNT; i++) {
        uint16_t x = CHARACTER_CARD_X + i * (CHARACTER_CARD_W + CHARACTER_CARD_GAP);

        if (selected_p1 == i) {
            draw_locked_character_overlay(x, CHARACTER_CARD_Y);
        } else if (selected_p2 == i) {
            draw_character_selection_marker(x, CHARACTER_CARD_Y, 0xD58C2A);
        }
    }
}

void video_draw_sudden_death_notice(int seconds, int frame_count) {
    const char *text = "SUDDEN DEATH";
    uint16_t scale = (frame_count < 15 || (frame_count >= 30 && frame_count < 45)) ? 7 : 6;
    uint16_t text_w = wide_text_width(text, scale);
    uint16_t text_h = 7 * scale;
    uint16_t x = (h_res - text_w) / 2;
    uint16_t y = 105;
    uint32_t color = (frame_count < 30) ? 0xFF3A2E : 0xFFFFFF;

    if (seconds >= 3) {
        return;
    }

    draw_rectangle_bb(x - 22, y - 18, text_w + 44, text_h + 36, 0x120508);
    draw_rectangle_bb(x - 16, y - 12, text_w + 32, text_h + 24, 0x381004);
    draw_wide_text(text, x + 3, y + 3, scale, 0x000000);
    draw_wide_text(text, x, y, scale, color);
}

void video_draw_hud(int score_left, int score_right, int seconds_left) {
    char hud[20];
    uint16_t scale = 5;
    uint16_t y = 24;

    if (seconds_left < 0) {
        int overtime_seconds = -seconds_left;
        snprintf(hud, sizeof(hud), "%d   +%02d:%02d   %d",
                 score_left, overtime_seconds / 60, overtime_seconds % 60, score_right);
    } else {
        snprintf(hud, sizeof(hud), "%d   %02d:%02d   %d",
                 score_left, seconds_left / 60, seconds_left % 60, score_right);
    }

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

void draw_xpm_flipped_horizontal(xpm_image_t img, uint16_t x, uint16_t y) {
    uint32_t *map = (uint32_t *)img.bytes;
    uint32_t transparent = xpm_transparency_color(img.type);

    for (uint16_t i = 0; i < img.height; i++) {
        for (uint16_t j = 0; j < img.width; j++) {
            uint32_t color = map[i * img.width + (img.width - 1 - j)];
            if (color != transparent) {
                vg_draw_pixel_bb(x + j, y + i, color);
            }
        }
    }
}
#pragma clang optimize on
