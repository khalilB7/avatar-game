#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FRAME_COUNT 3
#define DETECTION_RANGE 60

typedef struct {
    int x, y, w, h;
    int dx, speed, frame;
    int health;
    int isDead;
    int deathFrame;
    SDL_Texture *framesRight[FRAME_COUNT];
    SDL_Texture *attack;
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
void updateEnemy(Enemy *e, Player *p);

#endif
