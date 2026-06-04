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

    // 5. Net collision (Circle vs AABB with top/side differentiation)
    int net_width = 10;
    float net_top = FLOOR_Y - NET_HEIGHT;
    float net_bottom = FLOOR_Y;
    float net_left = NET_X - net_width / 2.0f;
    float net_right = NET_X + net_width / 2.0f;

    // Find the closest point on the net rectangle to the ball center
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
        // Determine if the ball hit the top or the side of the net.
        // Top collision: the ball was above the net top edge before impact.
        bool is_top_collision = (b->y - BALL_RADIUS) <= net_top &&
                                b->x >= net_left - BALL_RADIUS &&
                                b->x <= net_right + BALL_RADIUS;

        if (is_top_collision) {
            // Clamp the ball to sit exactly on top of the net
            b->y = net_top - BALL_RADIUS;
            // Small upward bounce so the ball doesn't get stuck
            b->vy = -3.0f;
            // Dampen forward momentum slightly — ball continues in its direction
            b->vx *= 0.8f;
        } else {
            // Side collision: push ball out and reverse horizontal velocity
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

            // Reverse horizontal velocity and apply bounce
            b->vx = -b->vx * BALL_BOUNCE;
        }
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

    // Find the closest point on the player rectangle to the ball center
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
        // Push ball outside player along penetration vector
        if (dist_sq > 0.0f) {
            // Ball center is outside the rectangle — use the collision normal
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
            // Ball center is inside the rectangle — fallback to axis-aligned push-out
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

        // Determine rebound direction from relative position
        // Positive offset = ball is to the right of the player center
        // Negative offset = ball is to the left
        float offset = b->x - p->x;
        float half_w = PLAYER_WIDTH / 2.0f;

        // Normalize offset to [-1, 1] range
        float norm = offset / half_w;
        if (norm > 1.0f)  norm = 1.0f;
        if (norm < -1.0f) norm = -1.0f;

        // Directional horizontal impulse based on where the ball hit
        // Base speed range: 2 (center hit) to 8 (edge hit)
        float base_speed = 2.0f + 6.0f * (norm < 0 ? -norm : norm);
        b->vx = (norm >= 0 ? 1.0f : -1.0f) * base_speed;
        b->vy = -12.0f;

        // Momentum transfer from player movement
        float momentum_weight = 0.5f;
        b->vx += p->vx * momentum_weight;
        b->vy += p->vy * momentum_weight;

        // Clamp velocities to keep physics stable
        float max_vx = 15.0f;
        float max_vy = 15.0f;

        if (b->vx > max_vx)  b->vx = max_vx;
        if (b->vx < -max_vx) b->vx = -max_vx;
        if (b->vy > max_vy)  b->vy = max_vy;
        if (b->vy < -max_vy) b->vy = -max_vy;
    }
}
