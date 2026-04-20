#include "enemy.h"
#include <stdlib.h>
#include <time.h>

int checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void updateEnemy(Enemy *e, Player *p) {
    if (e->health <= 0) {
        e->isDead = 1;
        e->dx = 0;
        e->deathFrame = (e->deathFrame + 1) % FRAME_COUNT;
        return;
    }

    int dx = p->x - e->x;
    int dy = p->y - e->y;

    if (abs(dx) < DETECTION_RANGE && abs(dy) < DETECTION_RANGE) {
        if (dx < 0) e->dx = -e->speed;
        else if (dx > 0) e->dx = e->speed;
    } else {
        if (rand() % 100 < 5) e->dx = -e->dx;
    }

    e->x += e->dx;

    if (e->x < 0) { e->x = 0; e->dx = e->speed; }
    if (e->x + e->w > 640) { e->x = 640 - e->w; e->dx = -e->speed; }

    e->frame = (e->frame + 1) % FRAME_COUNT;
}
