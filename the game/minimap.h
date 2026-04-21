#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define MINIMAP_X      580
#define MINIMAP_Y       20
#define MINIMAP_WIDTH  200
#define MINIMAP_HEIGHT 120

typedef struct {
    SDL_Rect     mapRect;
    SDL_Texture *mapTexture;

    /* One dot per tracked entity */
    SDL_Rect  dots[4];
    SDL_Color dotColors[4];
    int       numDots;
} Minimap;

void initMinimap(Minimap *m, SDL_Renderer *renderer);
void minimapClear(Minimap *m);
void minimapAddDot(Minimap *m, int worldX, int worldY,
                   int worldW, int worldH, SDL_Color color);
void renderMinimap(Minimap *m, SDL_Renderer *renderer);
void freeMinimap(Minimap *m);

int checkCollision(SDL_Rect a, SDL_Rect b);

#endif /* MINIMAP_H */
