#include "enemy.h"
#include <stdlib.h>

int checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void enemyTryAttackHit(Enemy *enemy, Player *p, SDL_Rect enemyRect, SDL_Rect playerRect) {

    if (enemy->isAttacking &&
        enemy->attackType == 1 &&
        enemy->canDamage &&
        !enemy->attackHit) {

        if (checkCollision(enemyRect, playerRect)) {
            p->health--;
            enemy->attackHit = 1;
        }
    }
}

void updateEnemy(Enemy *enemy, Player *p) {

    // DEAD
    if (enemy->health <= 0) {
        enemy->isDead = 1;
        enemy->dx = 0;
        enemy->dy = 0;

        if (enemy->deathFrame < FRAME_COUNT - 1)
            enemy->deathFrame++;

        return;
    }

    // ATTACK STATE
    if (enemy->isAttacking) {

        enemy->attackTimer--;
        enemy->attackFrame++;

        enemy->canDamage = 0;

        // ONLY attackpng damages
        if (enemy->attackType == 1) {
            if (enemy->attackFrame >= 3 && enemy->attackFrame <= 6) {
                enemy->canDamage = 1;
            }
        }

        if (enemy->attackTimer <= 0) {
            enemy->isAttacking = 0;
            enemy->attackFrame = 0;
            enemy->attackHit = 0;
            enemy->canDamage = 0;
        }

        return;
    }

    int dx = p->x - enemy->x;
    int dy = p->y - enemy->y;

    // ATTACK TRIGGER
    if (abs(dx) < 50 && abs(dy) < 50 && !enemy->isAttacking) {

        enemy->isAttacking = 1;
        enemy->attackTimer = 10;
        enemy->attackFrame = 0;

        enemy->dx = 0;
        enemy->dy = 0;

        enemy->attackType = (rand() % 100 < 30) ? 1 : 0;
        return;
    }

    // CHASE PLAYER
    if (abs(dx) < DETECTION_RANGE && abs(dy) < DETECTION_RANGE) {

        enemy->dx = (dx < 0) ? -enemy->speed : (dx > 0) ? enemy->speed : 0;
        enemy->dy = (dy < 0) ? -enemy->speed : (dy > 0) ? enemy->speed : 0;

    } else {
        if (rand() % 100 < 5) {
            enemy->dx = -enemy->dx;
            enemy->dy = -enemy->dy;
        }
    }

    enemy->x += enemy->dx;
    enemy->y += enemy->dy;

    // BOUNDS
    if (enemy->x < 0) { enemy->x = 0; enemy->dx = enemy->speed; }
    if (enemy->x + enemy->w > 640) { enemy->x = 640 - enemy->w; enemy->dx = -enemy->speed; }

    if (enemy->y < 0) { enemy->y = 0; enemy->dy = enemy->speed; }
    if (enemy->y + enemy->h > 480) { enemy->y = 480 - enemy->h; enemy->dy = -enemy->speed; }

    enemy->frame = (enemy->frame + 1) % FRAME_COUNT;
}
