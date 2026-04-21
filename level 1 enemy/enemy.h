#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FRAME_COUNT 3
#define DETECTION_RANGE 200

typedef struct {
    int x, y, w, h;
    int dx, dy, speed, frame;
    int health;
    int isDead;
    int deathFrame;

    int isAttacking;
    int attackTimer;
    int attackFrame;
    int attackHit;

    int attackType;   // 0 = normal, 1 = attackpng (damage)
    int canDamage;

    SDL_Texture *frames[FRAME_COUNT];
    SDL_Texture *attack;
    SDL_Texture *attack2;
    SDL_Texture *deathFrames[FRAME_COUNT];
} Enemy;

typedef struct {
    int x, y, w, h;
    int health;
} Player;

typedef struct {
    int x, y, w, h;
} Item;

int checkCollision(SDL_Rect a, SDL_Rect b);
void updateEnemy(Enemy *enemy, Player *p);
void enemyTryAttackHit(Enemy *enemy, Player *p, SDL_Rect enemyRect, SDL_Rect playerRect);

#endif
