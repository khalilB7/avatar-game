#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    int type; // 0 fixe, 1 P1, 2 P2
    SDL_Texture *texture; // <-- NEW: Holds the image for the object
} Platform;

typedef struct {
    SDL_Texture *background;
    SDL_Rect camera;
    int bg_w; // <-- NEW: To store background width
    int bg_h; // <-- NEW: To store background height

    Platform platforms[10];
    int nbPlatforms;

    Uint32 startTime;
    TTF_Font *font;
} Game;

int initGame(Game *g, SDL_Renderer *renderer);
void updateGame(Game *g);
void renderGame(Game *g, SDL_Renderer *renderer);
void handleScrolling(Game *g, SDL_Event e);
void renderTimer(Game *g, SDL_Renderer *renderer);
void renderGuide(SDL_Renderer *renderer, TTF_Font *font);

void freeGame(Game *g);

#endif
