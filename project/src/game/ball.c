/*
 * Ball physics and scoring.
 *
 * The game state machine owns when a rally starts or ends; this module owns how
 * the ball moves during a frame, how it reacts to the arena/net, and how player
 * contact changes its velocity.
 */
#include "ball.h"

#define BALL_RADIUS 20
#define BALL_BOUNCE 0.85f
#define BALL_START_SPEED_X 5.0f
#define BALL_START_SPEED_Y -8.0f

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

    /* Arena bounds are resolved before scoring so the ball stays on screen. */
    if (b->x - BALL_RADIUS < 0) {
        b->x = BALL_RADIUS;
        b->vx = -b->vx * BALL_BOUNCE;
    }

    if (b->x + BALL_RADIUS > H_RES) {
        b->x = H_RES - BALL_RADIUS;
        b->vx = -b->vx * BALL_BOUNCE;
    }

    if (b->y - BALL_RADIUS < 0) {
        b->y = BALL_RADIUS;
        b->vy = -b->vy * BALL_BOUNCE;
    }

    /*
     * Net collision uses circle-vs-AABB. The closest point gives both the
     * collision test and the push-out normal; top hits are handled separately so
     * the ball can rest/bounce on the net instead of being reflected sideways.
     */
    int net_width = 10;
    float net_top = FLOOR_Y - NET_HEIGHT;
    float net_bottom = FLOOR_Y;
    float net_left = NET_X - net_width / 2.0f;
    float net_right = NET_X + net_width / 2.0f;

    float closest_x = b->x;
    float closest_y = b->y;
    if (closest_x < net_left)   closest_x = net_left;
    if (closest_x > net_right)  closest_x = net_right;
    if (closest_y < net_top)    closest_y = net_top;
    if (closest_y > net_bottom) closest_y = net_bottom;

    float dx = b->x - closest_x;
    float dy = b->y - closest_y;
    float dist_sq = dx * dx + dy * dy;
    float radius_sq = (float)BALL_RADIUS * BALL_RADIUS;

    if (dist_sq < radius_sq && dist_sq > 0.0f) {
        bool is_top_collision = (b->y - BALL_RADIUS) <= net_top &&
                                b->x >= net_left - BALL_RADIUS &&
                                b->x <= net_right + BALL_RADIUS;

        if (is_top_collision) {
            b->y = net_top - BALL_RADIUS;
            b->vy = -3.0f;
            /* Preserve direction while reducing speed to avoid repeated overlap. */
            b->vx *= 0.8f;
        } else {
            /*
             * Fast inverse square root approximates 1/sqrt(dist_sq), which is
             * enough here to normalize the push-out vector without libm.
             */
            float x2 = dist_sq * 0.5f;
            int i = *(int*)&dist_sq;
            i = 0x5f3759df - (i >> 1);
            float inv_dist = *(float*)&i;
            inv_dist = inv_dist * (1.5f - x2 * inv_dist * inv_dist);

            float overlap = BALL_RADIUS - 1.0f / inv_dist;
            float nx = dx * inv_dist;
            float ny = dy * inv_dist;

            b->x += nx * overlap;
            b->y += ny * overlap;

            b->vx = -b->vx * BALL_BOUNCE;
        }
    }

    /* Touching the floor ends the rally; the x-position tells which side lost. */
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
    /*
     * Player collision is also circle-vs-AABB. The resolution step first moves
     * the ball out of the player hitbox, then computes a volley direction from
     * where the ball contacted the player's body.
     */
    float p_left = p->x - PLAYER_WIDTH / 2.0f;
    float p_right = p->x + PLAYER_WIDTH / 2.0f;
    float p_top = p->y - PLAYER_HEIGHT / 2.0f;
    float p_bottom = p->y + PLAYER_HEIGHT / 2.0f;

    float closest_x = b->x;
    float closest_y = b->y;
    if (closest_x < p_left)   closest_x = p_left;
    if (closest_x > p_right)  closest_x = p_right;
    if (closest_y < p_top)    closest_y = p_top;
    if (closest_y > p_bottom) closest_y = p_bottom;

    float dx = b->x - closest_x;
    float dy = b->y - closest_y;
    float dist_sq = dx * dx + dy * dy;
    float radius_sq = (float)BALL_RADIUS * BALL_RADIUS;

    if (dist_sq < radius_sq) {
        if (dist_sq > 0.0f) {
            float x2 = dist_sq * 0.5f;
            int i = *(int*)&dist_sq;
            i = 0x5f3759df - (i >> 1);
            float inv_dist = *(float*)&i;
            inv_dist = inv_dist * (1.5f - x2 * inv_dist * inv_dist);
            float dist = 1.0f / inv_dist;

            float overlap = BALL_RADIUS - dist;
            b->x += dx * inv_dist * overlap;
            b->y += dy * inv_dist * overlap;
        } else {
            /* Degenerate overlap: choose the shallowest axis to escape cleanly. */
            float overlap_left = b->x - p_left + BALL_RADIUS;
            float overlap_right = p_right - b->x + BALL_RADIUS;
            float overlap_top = b->y - p_top + BALL_RADIUS;
            float overlap_bottom = p_bottom - b->y + BALL_RADIUS;

            float min_overlap = overlap_left;
            int side = 0; // 0=left, 1=right, 2=top, 3=bottom

            if (overlap_right < min_overlap) { min_overlap = overlap_right; side = 1; }
            if (overlap_top < min_overlap) { min_overlap = overlap_top; side = 2; }
            if (overlap_bottom < min_overlap) { min_overlap = overlap_bottom; side = 3; }

            if (side == 0) b->x = p_left - BALL_RADIUS;
            else if (side == 1) b->x = p_right + BALL_RADIUS;
            else if (side == 2) b->y = p_top - BALL_RADIUS;
            else if (side == 3) b->y = p_bottom + BALL_RADIUS;
        }

        /*
         * Hits near the center produce a safer vertical bounce; edge hits produce
         * stronger horizontal velocity, making shot placement controllable.
         */
        float offset = b->x - p->x;
        float half_w = PLAYER_WIDTH / 2.0f;

        float norm = offset / half_w;
        if (norm > 1.0f)  norm = 1.0f;
        if (norm < -1.0f) norm = -1.0f;

        float base_speed = 2.0f + 6.0f * (norm < 0 ? -norm : norm);
        b->vx = (norm >= 0 ? 1.0f : -1.0f) * base_speed;
        b->vy = -12.0f;

        /* Add part of the player's movement so jumping/running affects volleys. */
        float momentum_weight = 0.5f;
        b->vx += p->vx * momentum_weight;
        b->vy += p->vy * momentum_weight;

        /* Clamp velocities to keep physics stable and avoid tunneling. */
        float max_vx = 15.0f;
        float max_vy = 15.0f;

        if (b->vx > max_vx)  b->vx = max_vx;
        if (b->vx < -max_vx) b->vx = -max_vx;
        if (b->vy > max_vy)  b->vy = max_vy;
        if (b->vy < -max_vy) b->vy = -max_vy;
    }
}
