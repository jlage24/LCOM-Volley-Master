#ifndef _GAME_H_
#define _GAME_H_

#include <stdbool.h>

#define VIDEO_MODE  0x118
#define H_RES       1024
#define V_RES       768
#define FLOOR_Y     680
#define NET_X       512
#define NET_HEIGHT  160
#define WIN_SCORE   5
#define GAME_TIME_SECONDS 60
#define GRAVITY     0.2f

#define CHARACTER_CARD_Y 385
#define CHARACTER_CARD_W 123
#define CHARACTER_CARD_H 210
#define CHARACTER_LOCK_Y 389

typedef enum {
    STATE_MENU,
    STATE_CHARACTER_SELECT,
    STATE_FIELD_SELECT,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_END
} GameState;

extern GameState current_state;

typedef enum e_field_type {
    FIELD_BEACH,
    FIELD_DESERT,
    FIELD_ICE,
    FIELD_JUNGLE,
    FIELD_COUNT
} t_field_type;

extern t_field_type selected_field;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
} Ball;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool facing_right;
    bool on_ground;
    int score;
} Player;

void game_init(void);
int game_update(void);
void game_render(void);
bool game_should_exit(void);

#endif
