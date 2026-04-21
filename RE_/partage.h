#ifndef GAME1_H
#define GAME1_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    int type; // 0 fixe, 1 P1 (Blue), 2 P2 (Red)
} Platform;

typedef struct {
    SDL_Texture *background;
    SDL_Rect camera1; // Camera for Player 1 (Left screen)
    SDL_Rect camera2; // Camera for Player 2 (Right screen)
    int bg_w;
    int bg_h;

    Platform platforms[10];
    int nbPlatforms;

    Uint32 startTime;
    TTF_Font *font;
} Game1;

int initGame1(Game1 *g, SDL_Renderer *renderer);
void updateGame1(Game1 *g);
void renderGame1(Game1 *g, SDL_Renderer *renderer);
void freeGame1(Game1 *g);

#endif
