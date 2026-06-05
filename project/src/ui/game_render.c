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

void video_draw_selection_box(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    const uint32_t color = 0xF2B51F;
    const uint16_t thickness = 4;

    draw_rectangle_bb(x, y, width, thickness, color);
    draw_rectangle_bb(x, y + height - thickness, width, thickness, color);
    draw_rectangle_bb(x, y, thickness, height, color);
    draw_rectangle_bb(x + width - thickness, y, thickness, height, color);
}

void video_draw_net() {
    /* Net hitbox hidden — uncomment to debug:
    uint16_t net_width = 10;
    uint16_t start_x = NET_X - (net_width / 2); 
    uint16_t start_y = FLOOR_Y - NET_HEIGHT;             
    draw_rectangle_bb(start_x, start_y, net_width, NET_HEIGHT, 0xFFFFFF);
    */
}

static bool xpm_is_valid(const xpm_image_t *img) {
    return img != NULL && img->bytes != NULL && img->width > 0 && img->height > 0;
}

void video_draw_ball(Ball *b) {
    if (b == NULL || !xpm_is_valid(&ball_pixmap)) {
        return;
    }

    uint16_t draw_x = (uint16_t)(b->x) - ball_pixmap.width / 2;
    uint16_t draw_y = (uint16_t)(b->y) - ball_pixmap.height / 2;
    
    draw_xpm(ball_pixmap, draw_x, draw_y);
}

void video_draw_players(Player *p1, Player *p2) {
    uint16_t width = PLAYER_WIDTH;
    uint16_t height = PLAYER_HEIGHT;
    
    if (p1 != NULL && xpm_is_valid(player1_pixmap)) {
        uint16_t p1_x = (uint16_t)(p1->x) - (width / 2);
        uint16_t p1_y = (uint16_t)(p1->y) - (height / 2);

        if (p1->facing_right) draw_xpm(*player1_pixmap, p1_x, p1_y);
        else draw_xpm_flipped_horizontal(*player1_pixmap, p1_x, p1_y);
    }
    
    if (p2 != NULL && xpm_is_valid(player2_pixmap)) {
        uint16_t p2_x = (uint16_t)(p2->x) - (width / 2);
        uint16_t p2_y = (uint16_t)(p2->y) - (height / 2);

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
    static const uint8_t slash[5] = {1, 1, 2, 4, 4};
    static const uint8_t plus[5] = {0, 2, 7, 2, 0};
    static const uint8_t dash[5] = {0, 0, 7, 0, 0};

    if (c >= '0' && c <= '9') {
        return digits[c - '0'][row];
    }
    if (c == ':') {
        return colon[row];
    }
    if (c == '/') {
        return slash[row];
    }
    if (c == '+') {
        return plus[row];
    }
    if (c == '-') {
        return dash[row];
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

static void draw_locked_character_overlay(uint16_t x, uint16_t y) {
    draw_xpm(locked_character_pixmap, x, y);
}

static void draw_character_selection_marker(uint16_t x, uint16_t y, uint32_t color) {
    draw_rectangle_bb(x, y, CHARACTER_CARD_W, 3, color);
    draw_rectangle_bb(x, y + CHARACTER_CARD_H - 3, CHARACTER_CARD_W, 3, color);
    draw_rectangle_bb(x, y, 3, CHARACTER_CARD_H, color);
    draw_rectangle_bb(x + CHARACTER_CARD_W - 3, y, 3, CHARACTER_CARD_H, color);
}

void video_draw_character_select(int selected_p1, int selected_p2, bool choosing_p2) {
    static const uint16_t character_card_x[PLAYER_CHARACTER_COUNT] = {
        112, 247, 381, 514, 647, 780
    };

    draw_xpm(character_select_pixmap, 0, 0);

    draw_rectangle_bb(350, 142, 330, 56, 0x4B260C);
    draw_centered_wide_text(choosing_p2 ? "PLAYER TWO" : "PLAYER ONE",
                            154, 5, 0xF2B51F);

    for (int i = 0; i < PLAYER_CHARACTER_COUNT; i++) {
        uint16_t x = character_card_x[i];

        if (selected_p1 == i) {
            draw_locked_character_overlay(x + 3, CHARACTER_LOCK_Y);
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

void video_draw_rtc_date(const rtc_date *date) {
    char date_text[9];
    char time_text[6];
    uint16_t scale = 4;
    uint16_t date_w;
    uint16_t time_w;
    uint16_t x;
    uint16_t y = 28;

    if (date == NULL) {
        return;
    }

    snprintf(date_text, sizeof(date_text), "%02u/%02u/%02u",
             date->day, date->month, date->year);
    snprintf(time_text, sizeof(time_text), "%02u:%02u",
             date->hours, date->minutes);

    date_w = text_width(date_text, scale);
    time_w = text_width(time_text, scale);
    x = h_res - date_w - 30;

    draw_rectangle_bb(x - 12, y - 10, date_w + 24, 58, 0x17120A);
    draw_text(date_text, x + 2, y + 2, scale, 0x000000);
    draw_text(date_text, x, y, scale, 0xFFFFFF);

    x = h_res - time_w - 30;
    draw_text(time_text, x + 2, y + 28, scale, 0x000000);
    draw_text(time_text, x, y + 26, scale, 0xF2B51F);
}

void video_draw_final_score(int score_p1, int score_p2) {
    const uint16_t card_w = 300;
    const uint16_t card_h = 34;
    const uint16_t card_x = (h_res - card_w) / 2;
    const uint16_t card_y = 160;
    const uint16_t border = 2;
    const uint16_t label_scale = 2;
    const uint16_t score_scale = 3;
    const char *label = "FINAL SCORE";
    char score[16];
    uint16_t label_w;
    uint16_t score_w;
    uint16_t score_h = 5 * score_scale;
    uint16_t group_w;
    uint16_t label_x;
    uint16_t label_y;
    uint16_t score_x;
    uint16_t score_y;

    snprintf(score, sizeof(score), "%d - %d", score_p1, score_p2);
    label_w = wide_text_width(label, label_scale);
    score_w = text_width(score, score_scale);
    group_w = label_w + 24 + score_w;
    label_x = card_x + (card_w - group_w) / 2;
    label_y = card_y + 10;
    score_x = label_x + label_w + 24;
    score_y = card_y + (card_h - score_h) / 2;

    draw_rectangle_bb(card_x, card_y, card_w, card_h, 0x12180F);
    draw_rectangle_bb(card_x + border, card_y + border,
                      card_w - (border * 2), card_h - (border * 2), 0x1E2B17);
    draw_rectangle_bb(card_x, card_y, card_w, border, 0x4B5F35);
    draw_rectangle_bb(card_x, card_y + card_h - border, card_w, border, 0x4B5F35);
    draw_rectangle_bb(card_x, card_y, border, card_h, 0x4B5F35);
    draw_rectangle_bb(card_x + card_w - border, card_y, border, card_h, 0x4B5F35);

    draw_wide_text(label, label_x + 1, label_y + 1, label_scale, 0x000000);
    draw_wide_text(label, label_x, label_y, label_scale, 0xB9C59A);
    draw_text(score, score_x + 2, score_y + 2, score_scale, 0x000000);
    draw_text(score, score_x, score_y, score_scale, 0xE0E6C8);
}

void video_draw_countdown(int value) {
    uint16_t scale;
    uint16_t text_w;
    uint16_t text_h;
    uint16_t x;
    uint16_t y;

    if (value > 0) {
        char digit[2];
        digit[0] = (char)('0' + value);
        digit[1] = '\0';
        scale = 28;
        text_w = text_width(digit, scale);
        text_h = 5 * scale;
        x = (h_res - text_w) / 2;
        y = (v_res - text_h) / 2;

        draw_rectangle_bb(x - 28, y - 24, text_w + 56, text_h + 48, 0x120508);
        draw_rectangle_bb(x - 20, y - 16, text_w + 40, text_h + 32, 0x381004);
        draw_text(digit, x + 4, y + 4, scale, 0x000000);
        draw_text(digit, x, y, scale, 0xFFFFFF);
    } else {
        const char *text = "START";
        scale = 12;
        text_w = wide_text_width(text, scale);
        text_h = 7 * scale;
        x = (h_res - text_w) / 2;
        y = (v_res - text_h) / 2;

        draw_rectangle_bb(x - 28, y - 24, text_w + 56, text_h + 48, 0x120508);
        draw_rectangle_bb(x - 20, y - 16, text_w + 40, text_h + 32, 0x381004);
        draw_wide_text(text, x + 4, y + 4, scale, 0x000000);
        draw_wide_text(text, x, y, scale, 0xFFFFFF);
    }
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
    if (!xpm_is_valid(&img)) {
        return;
    }

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
    if (!xpm_is_valid(&img)) {
        return;
    }

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
