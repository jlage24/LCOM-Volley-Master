#include "ball.h"

#define BALL_RADIUS 16
#define BALL_BOUNCE 0.75f
#define BALL_START_SPEED_X 5.0f
#define BALL_START_SPEED_Y -6.0f

void ball_init(Ball *b, int direction) {
    if (direction == 0) {
        direction = 1;
    }

    b->x = H_RES / 2.0f;
    b->y = 250.0f;
    b->vx = BALL_START_SPEED_X * direction;
    b->vy = BALL_START_SPEED_Y;
}

int ball_update(Ball *b) {
    b->vy += GRAVITY;
    
    b->x += b->vx;
    b->y += b->vy;

    // 1. Left wall collision
    if (b->x - BALL_RADIUS < 0) {
        b->x = BALL_RADIUS;
        b->vx = -b->vx * BALL_BOUNCE;
    }

    // 2. Right wall collision
    if (b->x + BALL_RADIUS > H_RES) {
        b->x = H_RES - BALL_RADIUS;
        b->vx = -b->vx * BALL_BOUNCE;
    }

    // 3. Ceiling collision
    if (b->y - BALL_RADIUS < 0) {
        b->y = BALL_RADIUS;
        b->vy = -b->vy * BALL_BOUNCE;
    }

    // 5. Net collision
    int net_width = 12;
    float net_top = FLOOR_Y - NET_HEIGHT;
    float net_bottom = FLOOR_Y;
    float net_left = NET_X - net_width / 2.0f;
    float net_right = NET_X + net_width / 2.0f;

    float b_left = b->x - BALL_RADIUS;
    float b_right = b->x + BALL_RADIUS;
    float b_top = b->y - BALL_RADIUS;
    float b_bottom = b->y + BALL_RADIUS;

    if (b_right > net_left && b_left < net_right &&
        b_bottom > net_top && b_top < net_bottom) {
        
        // Push ball outside the net
        if (b->x < NET_X) {
            b->x = net_left - BALL_RADIUS;
        } else {
            b->x = net_right + BALL_RADIUS;
        }
        
        // Reverse horizontal velocity and apply BALL_BOUNCE
        b->vx = -b->vx * BALL_BOUNCE;
    }

    // 6. Scoring detection
    if (b->y + BALL_RADIUS >= FLOOR_Y) {
        if (b->x < NET_X) {
            return -1;
        }
        if (b->x >= NET_X) {
            return 1;
        }
    }

    return 0;
}

void ball_collide_with_player(Ball *b, const Player *p, int direction) {
    float p_left = p->x - PLAYER_WIDTH / 2.0f;
    float p_right = p->x + PLAYER_WIDTH / 2.0f;
    float p_top = p->y - PLAYER_HEIGHT / 2.0f;
    float p_bottom = p->y + PLAYER_HEIGHT / 2.0f;

    float b_left = b->x - BALL_RADIUS;
    float b_right = b->x + BALL_RADIUS;
    float b_top = b->y - BALL_RADIUS;
    float b_bottom = b->y + BALL_RADIUS;

    if (b_right > p_left && b_left < p_right &&
        b_bottom > p_top && b_top < p_bottom) {
        
        // Place ball just outside player
        // To keep it simple and robust, we push the ball out along the shortest overlapping axis
        float overlap_left = b_right - p_left;
        float overlap_right = p_right - b_left;
        float overlap_top = b_bottom - p_top;
        float overlap_bottom = p_bottom - b_top;

        float min_overlap = overlap_left;
        int side = 0; // 0=left, 1=right, 2=top, 3=bottom

        if (overlap_right < min_overlap) { min_overlap = overlap_right; side = 1; }
        if (overlap_top < min_overlap) { min_overlap = overlap_top; side = 2; }
        if (overlap_bottom < min_overlap) { min_overlap = overlap_bottom; side = 3; }

        if (side == 0) b->x = p_left - BALL_RADIUS;
        else if (side == 1) b->x = p_right + BALL_RADIUS;
        else if (side == 2) b->y = p_top - BALL_RADIUS;
        else if (side == 3) b->y = p_bottom + BALL_RADIUS;
        
        b->vy = -9.0f;
        b->vx = 6.0f * direction;
    }
}
