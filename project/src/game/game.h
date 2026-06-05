#ifndef _GAME_H_
#define _GAME_H_

/*
 * Shared game model and public game-loop API.
 *
 * This header defines the court constants, state identifiers and lightweight
 * entity structs shared by gameplay, rendering and menu code. game.c owns the
 * mutable match state and exposes init/update/render entry points.
 */
#include <stdbool.h>

#define VIDEO_MODE  0x118
#define H_RES       1024
#define V_RES       768
#define FLOOR_Y     680
#define NET_X       512
#define NET_HEIGHT  300
#define WIN_SCORE   5
#define GAME_TIME_SECONDS 60
#define GRAVITY     0.2f

#define CHARACTER_CARD_Y 385
#define CHARACTER_CARD_W 123
#define CHARACTER_CARD_H 210
#define CHARACTER_LOCK_Y 389

typedef enum {
    /* High-level screens and phases handled by game_update/game_render. */
    STATE_MENU,
    STATE_CHARACTER_SELECT,
    STATE_FIELD_SELECT,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_END,
    STATE_HOW_TO_PLAY
} GameState;

extern GameState current_state;

typedef enum e_field_type {
    /* Field order matches the card indexes used by field selection. */
    FIELD_BEACH,
    FIELD_DESERT,
    FIELD_ICE,
    FIELD_JUNGLE,
    FIELD_COUNT
} t_field_type;

extern t_field_type selected_field;

typedef enum e_winner {
    WINNER_NONE,
    WINNER_PLAYER1,
    WINNER_PLAYER2
} t_winner;

typedef struct {
    /* Ball position is its center; velocity is measured in pixels per frame. */
    float x;
    float y;
    float vx;
    float vy;
} Ball;

typedef struct {
    /* Player position is the sprite/hitbox center used for collision/rendering. */
    float x;
    float y;
    float vx;
    float vy;
    bool facing_right;
    bool on_ground;
    int score;
} Player;

void game_init(void);
/* Advance one 60 Hz frame; returns non-zero when the main loop should stop. */
int game_update(void);
void game_render(void);
bool game_should_exit(void);

#endif
