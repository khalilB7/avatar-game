#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

/*
 * optionsmenu.h  -  Options-menu module.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include "gamestate.h"

typedef struct {
    TTF_Font    *font;

    SDL_Texture *volText;
    SDL_Rect     volRect;
    int          volumePercent;

    bool         skipBackground;  /* if true, skip drawing own bg */

    /* asset paths */
    const char  *pathBackground;
    const char  *pathFont;
    const char  *pathMusic;
    const char  *pathBtnNormal;
    const char  *pathBtnHover;
} OptionsMenu;

bool     OptionsMenu_Init        (OptionsMenu *o, SDL_Renderer *renderer);
AppState OptionsMenu_HandleEvent (OptionsMenu *o, SDL_Window *window,
                                  const SDL_Event *e, SDL_Renderer *renderer);
void     OptionsMenu_Update      (OptionsMenu *o);
void     OptionsMenu_Render      (OptionsMenu *o, SDL_Renderer *renderer);
void     OptionsMenu_Free        (OptionsMenu *o);

#endif /* OPTIONSMENU_H */
