#ifndef PARTAGE_H
#define PARTAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "hero.h"

/*
 * Split-screen subsystem.
 *
 * The screen is divided vertically:
 *   Left  (0..399)   → P1 viewport
 *   Right (400..799) → P2 viewport (visible only when P2 exists)
 *
 * Each viewport has its own camera that tracks its player.
 */

#define SPLIT_VIEW_W  400
#define SPLIT_VIEW_H  600

typedef struct {
    SDL_Texture *background;
    SDL_Rect     camera[2];   /* camera[0] → P1, camera[1] → P2 */
    int          bg_w, bg_h;

    Uint32    startTime;
    TTF_Font *font;
} SplitGame;

int  initSplitGame(SplitGame *g, SDL_Renderer *renderer);
void updateSplitGame(SplitGame *g, Hero *players, int numPlayers);
void renderSplitGame(SplitGame *g, SDL_Renderer *renderer,
                     Hero *players, int numPlayers, TTF_Font *font);
void freeSplitGame(SplitGame *g);

#endif /* PARTAGE_H */
