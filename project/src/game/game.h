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
#define GRAVITY     0.2f

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_END
} GameState;

extern GameState current_state;

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
    bool on_ground;
    int score;
} Player;

void game_init(void);
int game_update(void);
void game_render(void);
bool game_should_exit(void);

#endif
