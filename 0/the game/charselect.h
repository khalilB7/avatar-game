#ifndef CHARSELECT_H
#define CHARSELECT_H

/*
 * charselect.h  –  Character-selection screen.
 *
 * Shown when the player clicks "Play" from the main menu.
 * The user picks either Player 1 (soldier skin, arrow keys)
 * or Player 2 (C1 skin, ZQSD keys).
 *
 * After confirmation CharSelect_HandleEvent() returns STATE_PLAYING
 * and the caller reads cs->chosenId (1 or 2) to know which hero to
 * initialise.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "gamestate.h"

typedef struct {
    /* Result – read after STATE_PLAYING is returned */
    int chosenId;          /* 1 = soldier (arrows), 2 = C1 (ZQSD) */

    /* Internal */
    int  hovered;          /* 0 = none, 1 = left card, 2 = right card */
    int  selected;         /* 0 = none, 1 or 2 once confirmed        */

    SDL_Texture *preview[2];   /* sprite previews for card 1 and card 2 */
    SDL_Rect     cardRect[2];  /* bounding rect for each card            */
    SDL_Rect     confirmRect;  /* "Confirm" button                       */

    TTF_Font    *font;
    TTF_Font    *fontBig;

    bool         skipBackground;

    /* Asset paths – set before CharSelect_Init() */
    const char  *pathFont;
    const char  *pathFontBig;
    const char  *pathPreview1;   /* right-facing sprite sheet for hero 1 */
    const char  *pathPreview2;   /* right-facing sprite sheet for hero 2 */
    const char  *pathBg;
} CharSelect;

bool     CharSelect_Init        (CharSelect *cs, SDL_Renderer *renderer);
AppState CharSelect_HandleEvent (CharSelect *cs, const SDL_Event *e);
void     CharSelect_Update      (CharSelect *cs);
void     CharSelect_Render      (CharSelect *cs, SDL_Renderer *renderer);
void     CharSelect_Free        (CharSelect *cs);

#endif /* CHARSELECT_H */
