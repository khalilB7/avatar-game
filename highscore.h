#ifndef HIGHSCORE_H
#define HIGHSCORE_H

/*
 * highscore.h  -  High-score / name-input menu.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "gamestate.h"

#define HS_MAX_PLAYERS  100
#define HS_TOP_COUNT      5
#define HS_NAME_MAX      50

typedef struct {
    char   name[HS_NAME_MAX];
    int    score;
    Uint32 survivalSeconds;
} HS_Player;

typedef enum {
    HS_SCREEN_INPUT = 0,
    HS_SCREEN_BOARD = 1
} HS_Screen;

typedef struct {
    HS_Screen    screen;
    char         inputText[HS_NAME_MAX];
    int          pendingScore;
    Uint32       pendingSurvival;

    HS_Player    players[HS_MAX_PLAYERS];
    int          playerCount;

    TTF_Font    *font;
    TTF_Font    *fontBig;
    SDL_Texture *bgInput;
    SDL_Texture *bgScore;

    SDL_Rect     btnValider;
    SDL_Rect     btnRetour;

    bool         skipBackground;  /* if true, skip drawing own bg */

    const char  *pathFont;
    const char  *pathFontBig;
    const char  *pathBgInput;
    const char  *pathBgScore;
    const char  *pathScoreFile;
} HighScore;

bool     HighScore_Init           (HighScore *h, SDL_Renderer *renderer, int pendingScore);
AppState HighScore_HandleEvent    (HighScore *h, const SDL_Event *e, SDL_Renderer *renderer);
void     HighScore_Render         (HighScore *h, SDL_Renderer *renderer);
void     HighScore_Free           (HighScore *h);
void     HighScore_SetScore       (HighScore *h, int score);
void     HighScore_SetScoreAndTime(HighScore *h, int score, Uint32 survivalSeconds);

#endif /* HIGHSCORE_H */
