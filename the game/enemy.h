#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>



#define ENEMY_FRAME_COUNT  3
#define ENEMY_DETECTION    200   
#define MAX_ENEMIES         8

typedef struct {
    int x, y, w, h;
    int dx, speed;
    int frame;
    int health;
    int active;        

    int moveTimer;
    int animTimer;

    SDL_Texture *framesRight[ENEMY_FRAME_COUNT];
    SDL_Texture *attackTex;
} Enemy;


void initEnemy(Enemy *e, int x, int y, SDL_Renderer *renderer);
void freeEnemy(Enemy *e);


void moveEnemy(Enemy *e, int targetX, int targetY, int worldW);


void drawEnemy(Enemy *e, SDL_Renderer *renderer, int camX, int camY);

#endif
