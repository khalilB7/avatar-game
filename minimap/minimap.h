#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    SDL_Rect mapRect;
    SDL_Rect playerPoint;
    SDL_Texture *mapTexture;
} Minimap;

void initMinimap(Minimap *m, SDL_Renderer *renderer);
void updateMinimap(Minimap *m, SDL_Rect player);
void renderMinimap(Minimap *m, SDL_Renderer *renderer);
void freeMinimap(Minimap *m);

// Logic mta3 el-collision
int checkCollision(SDL_Rect a, SDL_Rect b);

#endif
