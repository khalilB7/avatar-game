#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_PLATFORMS 16

typedef struct {
    SDL_Rect     rect;
    int          type;        /* 0 = static, 1 = P1, 2 = P2 */
    SDL_Texture *texture;
} GamePlatform;

typedef struct {
    SDL_Texture *background;
    SDL_Rect     camera;
    int          bg_w, bg_h;

    GamePlatform platforms[MAX_PLATFORMS];
    int          nbPlatforms;

    Uint32    startTime;
    TTF_Font *font;
} Game;

int  initGame(Game *g, SDL_Renderer *renderer);
void updateGame(Game *g, int followerX, int followerY, int followerW, int followerH);
void renderGame(Game *g, SDL_Renderer *renderer);
void renderTimer(Game *g, SDL_Renderer *renderer);
void freeGame(Game *g);

#endif /* GAME_H */
