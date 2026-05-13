#ifndef SAVEMENU_H
#define SAVEMENU_H

/*
 * savemenu.h  -  Save/Load menu module.
 *
 * Displays up to SAVE_SLOT_COUNT slots.  Each occupied slot shows:
 *   • Character name  (Soldier / Warrior)
 *   • Survival time   (MM:SS)
 *   • Best score
 * Layout uses proportional fractions of the renderer output size so it
 * scales correctly between 800×600 windowed and fullscreen modes.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "gamestate.h"
#include "savesystem.h"

typedef struct {
    SDL_Texture *texBg;
    SDL_Rect     slotRects[SAVE_SLOT_COUNT];
    SDL_Rect     btnBack;
    SaveSlotInfo slots[SAVE_SLOT_COUNT];
    int          pendingSlot;   /* slot chosen for load (-1 = none) */
    int          hoveredSlot;   /* slot under mouse (-1 = none)     */
    TTF_Font    *font;
    TTF_Font    *fontBig;

    bool         skipBackground;

    /* Asset paths – set before SaveMenu_Init() */
    const char  *pathTexBg;
    const char  *pathFont;
} SaveMenu;

bool     SaveMenu_Init        (SaveMenu *s, SDL_Renderer *renderer);
void     SaveMenu_Refresh     (SaveMenu *s);
AppState SaveMenu_HandleEvent (SaveMenu *s, const SDL_Event *e);
void     SaveMenu_Render      (SaveMenu *s, SDL_Renderer *renderer);
void     SaveMenu_Free        (SaveMenu *s);

#endif /* SAVEMENU_H */
