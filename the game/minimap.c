#include "minimap.h"
#include <string.h>

void initMinimap(Minimap *m, SDL_Renderer *renderer)
{
    memset(m, 0, sizeof(Minimap));
    m->mapRect = (SDL_Rect){MINIMAP_X, MINIMAP_Y, MINIMAP_WIDTH, MINIMAP_HEIGHT};

    SDL_Surface *surf = IMG_Load("/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/MiniMap.jpeg");
    if (surf) {
        m->mapTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

void minimapClear(Minimap *m)
{
    m->numDots = 0;
}

void minimapAddDot(Minimap *m, int worldX, int worldY,
                   int worldW, int worldH, SDL_Color color)
{
    if (m->numDots >= 4) return;
    float sx = (float)m->mapRect.w / (float)worldW;
    float sy = (float)m->mapRect.h / (float)worldH;
    int   i  = m->numDots;
    m->dots[i] = (SDL_Rect){
        m->mapRect.x + (int)(worldX * sx),
        m->mapRect.y + (int)(worldY * sy),
        8, 8
    };
    m->dotColors[i] = color;
    m->numDots++;
}

void renderMinimap(Minimap *m, SDL_Renderer *renderer)
{
    if (m->mapTexture) {
        SDL_RenderCopy(renderer, m->mapTexture, NULL, &m->mapRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 200);
        SDL_RenderFillRect(renderer, &m->mapRect);
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderDrawRect(renderer, &m->mapRect);
    }

    for (int i = 0; i < m->numDots; i++) {
        SDL_Color c = m->dotColors[i];
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &m->dots[i]);
    }
}

void freeMinimap(Minimap *m)
{
    if (m->mapTexture) SDL_DestroyTexture(m->mapTexture);
}

/* ── AABB helper – defined exactly once here ───────────────────────── */
int checkCollision(SDL_Rect a, SDL_Rect b)
{
    return (a.x         < b.x + b.w &&
            a.x + a.w   > b.x       &&
            a.y         < b.y + b.h &&
            a.y + a.h   > b.y);
}
