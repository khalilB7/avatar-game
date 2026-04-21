#include "minimap.h"

void initMinimap(Minimap *m, SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load("MiniMap.jpeg");
    m->mapTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    m->mapRect.x = 580; // Blaset-ha top-right
    m->mapRect.y = 20;
    m->mapRect.w = 200;
    m->mapRect.h = 120;
}

void updateMinimap(Minimap *m, SDL_Rect player) {
    // Calcul de position sur la minimap
    float scaleX = (float)m->mapRect.w / 800;
    float scaleY = (float)m->mapRect.h / 600;

    m->playerPoint.x = m->mapRect.x + (player.x * scaleX);
    m->playerPoint.y = m->mapRect.y + (player.y * scaleY);
    m->playerPoint.w = 5;
    m->playerPoint.h = 5;
}

void renderMinimap(Minimap *m, SDL_Renderer *renderer) {
    SDL_RenderCopy(renderer, m->mapTexture, NULL, &m->mapRect);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // No9ta 7amra
    SDL_RenderFillRect(renderer, &m->playerPoint);
}

void freeMinimap(Minimap *m) {
    SDL_DestroyTexture(m->mapTexture);
}

int checkCollision(SDL_Rect a, SDL_Rect b) {
    // AABB Collision Detection
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}
