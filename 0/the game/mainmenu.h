#ifndef MAINMENU_H
#define MAINMENU_H

/*
 * mainmenu.h  -  Main-menu module.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include "gamestate.h"

/* Button indices */
#define MM_BTN_PLAY       0
#define MM_BTN_OPTIONS    1
#define MM_BTN_SAVE       2
#define MM_BTN_HIGHSCORE  3
#define MM_BTN_EXIT       4
#define MM_BTN_COUNT      5

typedef struct {
    SDL_Rect    rect;
    const char *text;
    bool        hovered;
} MM_Button;

typedef struct {
    SDL_Texture *bgTexture;
    SDL_Texture *btnTexture;
    SDL_Texture *cornerTexture;
    Mix_Music   *bgMusic;
    Mix_Chunk   *hoverSound;
    TTF_Font    *font;
    MM_Button    buttons[MM_BTN_COUNT];

    bool         skipBackground;  /* if true, skip drawing own bg */

    /* asset paths - set before calling MainMenu_Init */
    const char  *pathBg;
    const char  *pathBtn;
    const char  *pathCorner;
    const char  *pathMusic;
    const char  *pathHoverSfx;
    const char  *pathFont;
} MainMenu;

bool     MainMenu_Init        (MainMenu *m, SDL_Renderer *renderer);
AppState MainMenu_HandleEvent (MainMenu *m, const SDL_Event *e);
void     MainMenu_Update      (MainMenu *m);
void     MainMenu_Render      (MainMenu *m, SDL_Renderer *renderer);
void     MainMenu_Free        (MainMenu *m);

#endif /* MAINMENU_H */
